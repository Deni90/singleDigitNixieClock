/******************************************************************************
 * File:    nixie_clock.cpp
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Implements the logic of the project
 ******************************************************************************/

#include "nixie_clock.h"

#include <cstring>

#include "cJSON.h"
#include "dns_server.h"
#include "driver/gpio.h"
#include "esp_littlefs.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_system.h"   //esp_init funtions esp_err_t
#include "esp_wifi.h"     //esp_wifi_init functions and wifi operations
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/inet.h"
#include "mbedtls/base64.h"
#include "mdns.h"
#include "nvs_flash.h"   //non volatile storage

#include "config_store.h"
#include "wifi_info.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char* kTag = "nixie_clock";
static const char* kApSsid = "NixieClock";
static constexpr int kMaxConnectionRetry = 5;
static constexpr gpio_num_t kLedPin = GPIO_NUM_15;
static constexpr gpio_num_t kBcdPinA = GPIO_NUM_19;
static constexpr gpio_num_t kBcdPinB = GPIO_NUM_18;
static constexpr gpio_num_t kBcdPinC = GPIO_NUM_17;
static constexpr gpio_num_t kBcdPinD = GPIO_NUM_16;
static constexpr u_int32_t kLedControllerUpdatePeriod = 4;   // ms 255* 4 = ~1s
static constexpr u_int32_t kTimeUpdatePeriod = 60000;        // 1 minute
static constexpr u_int32_t kDigitDuration = 300;
static constexpr u_int32_t kPauseDuration = 2000;
static constexpr u_int32_t kIntroDigitPeriod = 50;
static constexpr u_int8_t kCurrentTimeRepeatTimes = 3;
static const char* kNtpServerAddr = "pool.ntp.org";
static constexpr uint32_t kNtpSyncInterval = 3600000;   // 1 hour

NixieClock::NixieClock()
    : mLedController(kLedPin),
      mNixieTube(kBcdPinA, kBcdPinB, kBcdPinC, kBcdPinD), mWebServer(*this),
      mShowCurrentTimeTaskHandle(nullptr) {}

void NixieClock::initialize() {
    ESP_LOGI(kTag, "Initialize Nixie tube...");
    mNixieTube.initialize();
    ESP_LOGI(kTag, "Initialize Nixie tube... done");

    ESP_LOGI(kTag, "Initialize LittleFS...");
    if (setupLittlefs()) {
        ESP_LOGI(kTag, "Initialize LittleFS... done");
    } else {
        ESP_LOGE(kTag, "Initialize LittleFS... failed");
    }

    ESP_LOGI(kTag, "Initialize Config store...");
    ConfigStore::initialize();
    ESP_LOGI(kTag, "Initialize Config store... done");

    ESP_LOGI(kTag, "Initialize Led controller...");
    mLedController.initialize(ConfigStore::loadLedInfo().value_or(LedInfo()));
    ESP_LOGI(kTag, "Initialize Led controller... done");

    WifiInfo wifiInfo = ConfigStore::loadWifiInfo().value_or(WifiInfo());

    ESP_LOGI(kTag, "Initialize Wifi...");
    auto wifiMode = setupWifi(wifiInfo);
    ESP_LOGI(kTag, "Initialize Wifi... done");

    ESP_LOGI(kTag, "Initialize MDNS service...");
    startMdnsService(wifiInfo);
    ESP_LOGI(kTag, "Initialize MDNS service... done");

    ESP_LOGI(kTag, "Initialize Web server...");
    mWebServer.initialize();
    ESP_LOGI(kTag, "Initialize Web server... done");

    mSleepInfo = ConfigStore::loadSleepInfo().value_or(SleepInfo());

    // TODO initialize RTC

    ESP_LOGI(kTag, "Setting up time zone...");
    TimeInfo timeInfo = ConfigStore::loadTimeInfo().value_or(TimeInfo());
    ESP_LOGI(kTag, "Time zone: %s", timeInfo.getTzOffset().c_str());
    setenv("TZ", timeInfo.getTzOffset().c_str(), 1);
    tzset();
    ESP_LOGI(kTag, "Setting up time zone... done");

    // Initialize NTP only if the device is connected to some external wifi
    // network
    if (wifiMode == WifiMode::Sta) {
        ESP_LOGI(kTag, "Initialize SNTP...");
        initializeSNTP();
        ESP_LOGI(kTag, "Initialize SNTP... done");

        ESP_LOGI(kTag, "Synchronizing time with NTP server...");
        int retry = 0;
        const int retry_count = 10;
        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET &&
               ++retry < retry_count) {
            ESP_LOGI(kTag, "Waiting for system time to be set... (%d/%d)",
                     retry, retry_count);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(kTag, "Synchronizing time with NTP server... Done");
        // Show current time after NTP sync
        startShowCurrentTimeTask();
    } else {
        // TODO backup to RTC
    }

    handleSleepMode();

    xTaskCreate(loopTask, "loopTask", 2048, this, 2, nullptr);
}

std::optional<LedInfo> NixieClock::onGetLedInfo() const {
    return ConfigStore::loadLedInfo();
}

void NixieClock::onSetLedInfo(const LedInfo& ledInfo) {
    if (!isInSleepMode()) {
        mLedController.setLedInfo(ledInfo);
    }
    ConfigStore::saveLedInfo(ledInfo);
}

std::optional<SleepInfo> NixieClock::onGetSleepInfo() const {
    return ConfigStore::loadSleepInfo();
}

void NixieClock::onSetSleepInfo(const SleepInfo& sleepInfo) {
    mSleepInfo = sleepInfo;
    ConfigStore::saveSleepInfo(sleepInfo);
    handleSleepMode();
}

std::optional<WifiInfo> NixieClock::onGetWifiInfo() const {
    return ConfigStore::loadWifiInfo();
}

void NixieClock::onSetWifiInfo(const WifiInfo& wifiInfo) {
    ConfigStore::saveWifiInfo(wifiInfo);
    esp_restart();
}

std::optional<TimeInfo> NixieClock::onGetTimeInfo() const {
    return ConfigStore::loadTimeInfo();
}

void NixieClock::onSetTimeInfo(const TimeInfo& timeInfo) {
    ConfigStore::saveTimeInfo(timeInfo);
    setenv("TZ", timeInfo.getTzOffset().c_str(), 1);
    tzset();
    LedInfo ledInfo = ConfigStore::loadLedInfo().value();
    if (isInSleepMode()) {
        // turn off the led
        ledInfo.setState(LedState::Off);
    }
    mLedController.setLedInfo(ledInfo);
}

static EventGroupHandle_t gWifiEventGroup;

extern "C" void wifiEventHandler(void* arg, esp_event_base_t eventBase,
                                 int32_t eventId, void* eventData) {
    static int retryCount = 0;
    if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_START) {
        esp_wifi_connect();

    } else if (eventBase == WIFI_EVENT &&
               eventId == WIFI_EVENT_STA_DISCONNECTED) {
        if (retryCount < kMaxConnectionRetry) {
            esp_wifi_connect();
            retryCount++;
            ESP_LOGI(kTag, "Retrying connection to Wi-Fi...");
        } else {
            xEventGroupSetBits(gWifiEventGroup, WIFI_FAIL_BIT);
        }

    } else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = static_cast<ip_event_got_ip_t*>(eventData);
        ESP_LOGI(kTag, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        retryCount = 0;
        xEventGroupSetBits(gWifiEventGroup, WIFI_CONNECTED_BIT);
    }
}

bool NixieClock::setupLittlefs() {
    esp_vfs_littlefs_conf_t conf = {};
    conf.base_path = "/littlefs";
    conf.partition_label = "littlefs";
    conf.format_if_mount_failed = true;
    conf.dont_mount = false;

    // Use settings defined above to initialize and mount LittleFS filesystem.
    // Note: esp_vfs_littlefs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(kTag, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(kTag, "Failed to find LittleFS partition");
        } else {
            ESP_LOGE(kTag, "Failed to initialize LittleFS (%s)",
                     esp_err_to_name(ret));
        }
        return false;
    }
    return true;
}

NixieClock::WifiMode NixieClock::setupWifi(const WifiInfo& wifiInfo) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    gWifiEventGroup = xEventGroupCreate();
    if (wifiInfo.getSSID() == "" || !initializeWifiInStationMode(wifiInfo)) {
        initializeWifiInApMode(wifiInfo);
        ESP_LOGI(kTag, "Setup captive portal...");
        setupCaptivePortal();
        ESP_LOGI(kTag, "Setup captive portal... done");
        ESP_LOGI(kTag, "Start DNS server...");
        // Start the DNS server that will redirect all queries to the softAP IP
        dns_server_config_t config = DNS_SERVER_CONFIG_SINGLE(
            "*" /* all A queries */, "WIFI_AP_DEF" /* softAP netif ID */);
        start_dns_server(&config);
        ESP_LOGI(kTag, "Start DNS server... done");
        return WifiMode::Ap;
    }
    return WifiMode::Sta;
}

bool NixieClock::initializeWifiInStationMode(const WifiInfo& wifiInfo) {
    esp_netif_t* netif = esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(
        esp_netif_set_hostname(netif, wifiInfo.getHostname().c_str()));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler, nullptr, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifiEventHandler, nullptr, nullptr));

    wifi_config_t wifiConfig = {};
    std::strncpy(reinterpret_cast<char*>(wifiConfig.sta.ssid),
                 wifiInfo.getSSID().c_str(), sizeof(wifiConfig.sta.ssid));
    unsigned char password[64];
    size_t passwordLenght;
    mbedtls_base64_decode(password, 64, &passwordLenght,
                          (unsigned char*) wifiInfo.getPassword().c_str(),
                          wifiInfo.getPassword().length());
    password[passwordLenght] = '\0';
    std::strncpy(reinterpret_cast<char*>(wifiConfig.sta.password),
                 reinterpret_cast<char*>(password),
                 sizeof(wifiConfig.sta.password));
    wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifiConfig.sta.pmf_cfg.capable = true;
    wifiConfig.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits =
        xEventGroupWaitBits(gWifiEventGroup, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                            pdFALSE, pdFALSE, pdMS_TO_TICKS(10000));

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(kTag, "Connected to STA: %s", wifiInfo.getSSID().c_str());
        return true;
    }

    ESP_LOGW(kTag, "Failed to connect to STA.");
    return false;
}

void NixieClock::initializeWifiInApMode(const WifiInfo& wifiInfo) {
    esp_netif_t* netif = esp_netif_create_default_wifi_ap();
    ESP_ERROR_CHECK(
        esp_netif_set_hostname(netif, wifiInfo.getHostname().c_str()));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifiConfig = {};
    std::strncpy(reinterpret_cast<char*>(wifiConfig.ap.ssid), kApSsid,
                 sizeof(wifiConfig.ap.ssid));
    wifiConfig.ap.ssid_len = std::strlen(kApSsid);
    wifiConfig.ap.channel = 1;
    wifiConfig.ap.max_connection = 4;
    wifiConfig.ap.authmode = WIFI_AUTH_OPEN;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_start());

    esp_netif_ip_info_t ipInfo;
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"),
                          &ipInfo);

    ESP_LOGI(kTag, "Started AP mode with SSID: %s", wifiConfig.ap.ssid);
}

void NixieClock::setupCaptivePortal() {
    // get the IP of the access point to redirect to
    esp_netif_ip_info_t ipInfo;
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"),
                          &ipInfo);

    char ipAddr[16];
    inet_ntoa_r(ipInfo.ip.addr, ipAddr, 16);
    ESP_LOGI(kTag, "Set up softAP with IP: %s", ipAddr);

    // turn the IP into a URI
    char* captiveportal_uri = (char*) malloc(32 * sizeof(char));
    assert(captiveportal_uri && "Failed to allocate captiveportal_uri");
    strcpy(captiveportal_uri, "http://");
    strcat(captiveportal_uri, ipAddr);

    // get a handle to configure DHCP with
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");

    // set the DHCP option 114
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_stop(netif));
    ESP_ERROR_CHECK(esp_netif_dhcps_option(
        netif, ESP_NETIF_OP_SET, ESP_NETIF_CAPTIVEPORTAL_URI, captiveportal_uri,
        strlen(captiveportal_uri)));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_start(netif));
}

void NixieClock::startMdnsService(const WifiInfo& wifiInfo) {
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set(wifiInfo.getHostname().c_str()));
    ESP_ERROR_CHECK(mdns_instance_name_set("NixieClock web server"));
    ESP_LOGI(kTag, "mDNS started: http://%s.local",
             wifiInfo.getHostname().c_str());
}

void NixieClock::initializeSNTP() {
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, kNtpServerAddr);
    esp_sntp_set_sync_interval(kNtpSyncInterval);
    esp_sntp_init();
}

bool NixieClock::isInSleepMode() {
    time_t now;
    struct tm t;
    time(&now);
    localtime_r(&now, &t);
    int32_t timeInMinutes = t.tm_hour * 60 + t.tm_min;
    if (mSleepInfo.getSleepBefore() < mSleepInfo.getSleepAfter()) {
        if (timeInMinutes < mSleepInfo.getSleepBefore() ||
            timeInMinutes > mSleepInfo.getSleepAfter()) {
            return true;
        }
    } else {
        if (timeInMinutes < mSleepInfo.getSleepBefore() &&
            timeInMinutes > mSleepInfo.getSleepAfter()) {
            return true;
        }
    }
    return false;
}

void NixieClock::loopTask(void* param) {
    NixieClock* self = static_cast<NixieClock*>(param);
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(1);   // 1 ms tick
    uint32_t msCounter = 0;
    int32_t lastSecond = -1;

    while (true) {
        vTaskDelayUntil(&lastWakeTime, period);   // 1 ms precision
        msCounter++;
        // handle LED controller stuff here
        if (msCounter % kLedControllerUpdatePeriod == 0) {
            self->mLedController.update();
        }
        // handle time and nixie clock stuff here
        time_t now;
        struct tm timeInfo;
        time(&now);
        localtime_r(&now, &timeInfo);
        if (timeInfo.tm_sec == 0 && lastSecond != 0) {
            self->startShowCurrentTimeTask();
        }
        lastSecond = timeInfo.tm_sec;
    }
}

bool NixieClock::startShowCurrentTimeTask(void) {
    // check if task is already running, if yes do not create another one
    if (mShowCurrentTimeTaskHandle) {
        return false;
    }
    xTaskCreate(showCurrentTimeTask, "showCurrentTimeTask", 4096, this, 5,
                &mShowCurrentTimeTaskHandle);
    return true;
}

void NixieClock::showCurrentTimeTask(void* param) {
    NixieClock* self = static_cast<NixieClock*>(param);
    time_t now;
    struct tm timeInfo;
    time(&now);
    localtime_r(&now, &timeInfo);
    char buf[32];
    strftime(buf, sizeof(buf), "%H:%M:%S", &timeInfo);
    ESP_LOGI(kTag, "Current time: %s", buf);
    if (self->isInSleepMode()) {
        ESP_LOGI(kTag, "Sleep mode on");
        self->mShowCurrentTimeTaskHandle = nullptr;
        vTaskDelete(nullptr);
        return;
    }
    LedInfo ledInfo = ConfigStore::loadLedInfo().value_or(LedInfo());
    if (ledInfo.getState() != LedState::Off) {
        ledInfo.setState(LedState::On);
        self->mLedController.setLedInfo(ledInfo);
    }
    self->mLedController.lock();
    // Show a fast countdown of digits from 9 to 0 on nixie tube as an intro
    // before showing the actual time
    for (auto i = 9; i >= 0; --i) {
        self->mNixieTube.showDigit(i);
        vTaskDelay(kIntroDigitPeriod / portTICK_PERIOD_MS);
    }
    self->mNixieTube.hideDigit();
    vTaskDelay(kDigitDuration / portTICK_PERIOD_MS);
    // Show time on nixie tube. Since there is only one nixie tube it is
    // needed to sequentially show digits. The time is displayed in the
    // following format: H <pause> H <pause> M <pause> M
    int32_t nixieOutput[] = {timeInfo.tm_hour / 10, timeInfo.tm_hour % 10,
                             timeInfo.tm_min / 10, timeInfo.tm_min % 10};
    for (auto i = 0; i < kCurrentTimeRepeatTimes; ++i) {
        for (auto j = 0; j < sizeof(nixieOutput) / sizeof(nixieOutput[0]);
             ++j) {
            self->mNixieTube.showDigit(nixieOutput[j]);
            vTaskDelay(kDigitDuration / portTICK_PERIOD_MS);
            self->mNixieTube.hideDigit();
            if (j < sizeof(nixieOutput) / sizeof(nixieOutput[0] - 1)) {
                vTaskDelay(kDigitDuration / portTICK_PERIOD_MS);
            }
        }
        if (i < kCurrentTimeRepeatTimes - 1) {
            vTaskDelay(kPauseDuration / portTICK_PERIOD_MS);
        }
    }
    self->mLedController.unlock();
    ledInfo = ConfigStore::loadLedInfo().value_or(LedInfo());
    self->mLedController.setLedInfo(ledInfo);
    self->mShowCurrentTimeTaskHandle = nullptr;
    vTaskDelete(nullptr);
}
void NixieClock::handleSleepMode() {
    LedInfo ledInfo = ConfigStore::loadLedInfo().value();
    if (isInSleepMode()) {
        ESP_LOGI(kTag, "In sleep mode, turning off LED.");
        // turn off the led
        ledInfo.setState(LedState::Off);
    }
    mLedController.setLedInfo(ledInfo);
}