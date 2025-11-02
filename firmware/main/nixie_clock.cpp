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
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_system.h"   //esp_init funtions esp_err_t
#include "esp_wifi.h"     //esp_wifi_init functions and wifi operations
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/inet.h"
#include "mdns.h"

#include "config_store.h"
#include "wifi_info.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char* kTag = "nixie_clock";
static const char* kApHostname = "mynixieclock";
static const char* kApSsid = "NixieClock";
static constexpr gpio_num_t kLedPin = GPIO_NUM_15;
static constexpr gpio_num_t kBcdPinA = GPIO_NUM_19;
static constexpr gpio_num_t kBcdPinB = GPIO_NUM_18;
static constexpr gpio_num_t kBcdPinC = GPIO_NUM_17;
static constexpr gpio_num_t kBcdPinD = GPIO_NUM_16;
static constexpr i2c_port_t kI2cPort = I2C_NUM_0;
static constexpr gpio_num_t kI2cSda = GPIO_NUM_22;
static constexpr gpio_num_t kI2cScl = GPIO_NUM_23;
static constexpr u_int32_t kLedControllerUpdatePeriod = 4;   // ms 255* 4 = ~1s
static constexpr u_int32_t kTimeUpdatePeriod = 60000;        // 1 minute
static constexpr u_int32_t kDigitDuration = 300;
static constexpr u_int32_t kPauseDuration = 2000;
static constexpr u_int32_t kIntroDigitPeriod = 50;
static constexpr u_int8_t kCurrentTimeRepeatTimes = 3;
static const char* kNtpServerAddr = "pool.ntp.org";
static constexpr uint32_t kNtpSyncInterval = 3600000;   // 1 hour

static Ds3231* gRtcPtr = nullptr;

NixieClock::NixieClock()
    : mLedController(kLedPin),
      mNixieTube(kBcdPinA, kBcdPinB, kBcdPinC, kBcdPinD), mWebServer(*this),
      mShowCurrentTimeTaskHandle(nullptr), mI2c(kI2cPort, kI2cSda, kI2cScl),
      mRtc(mI2c), mLastSleepModeStatus(false) {
    gRtcPtr = &mRtc;
}

void NixieClock::initialize() {
    ESP_LOGI(kTag, "Initialize I2C interface...");
    mI2c.initialize();
    ESP_LOGI(kTag, "Initialize I2C interface... done");

    ESP_LOGI(kTag, "Initialize RTC clock...");
    mRtc.initialize();
    ESP_LOGI(kTag, "Initialize RTC clock... done");

    ESP_LOGI(kTag, "Initialize Nixie tube...");
    mNixieTube.initialize();
    ESP_LOGI(kTag, "Initialize Nixie tube... done");

    ESP_LOGI(kTag, "Initialize Config store...");
    ConfigStore::initialize();
    ESP_LOGI(kTag, "Initialize Config store... done");

    ESP_LOGI(kTag, "Initialize Led controller...");
    mLedController.initialize(ConfigStore::loadLedInfo().value_or(LedInfo()));
    ESP_LOGI(kTag, "Initialize Led controller... done");

    WifiInfo wifiInfo = ConfigStore::loadWifiInfo().value_or(WifiInfo());

    ESP_LOGI(kTag, "Initialize Wifi...");
    mWifiManager.initialize();
    ESP_LOGI(kTag, "Initialize Wifi... done");

    if (wifiInfo.getSSID() == "" || !mWifiManager.connectSta(wifiInfo)) {
        WifiInfo apWifiInfo(kApHostname, kApSsid, WifiAuthType::Open, "");
        mWifiManager.startAp(apWifiInfo);
        ESP_LOGI(kTag, "Setup captive portal...");
        setupCaptivePortal();
        ESP_LOGI(kTag, "Setup captive portal... done");
        ESP_LOGI(kTag, "Start DNS server...");
        // Start the DNS server that will redirect all queries to the softAP IP
        dns_server_config_t config = DNS_SERVER_CONFIG_SINGLE(
            "*" /* all A queries */, "WIFI_AP_DEF" /* softAP netif ID */);
        start_dns_server(&config);
        ESP_LOGI(kTag, "Start DNS server... done");
    }

    ESP_LOGI(kTag, "Initialize MDNS service...");
    startMdnsService(wifiInfo);
    ESP_LOGI(kTag, "Initialize MDNS service... done");

    ESP_LOGI(kTag, "Initialize Web server...");
    mWebServer.initialize();
    ESP_LOGI(kTag, "Initialize Web server... done");

    mSleepInfo = ConfigStore::loadSleepInfo().value_or(SleepInfo());

    ESP_LOGI(kTag, "Setting up time zone...");
    mTimeInfo = ConfigStore::loadTimeInfo().value_or(TimeInfo());
    ESP_LOGI(kTag, "Time zone: %s", mTimeInfo.getTzOffset().c_str());
    setenv("TZ", mTimeInfo.getTzOffset().c_str(), 1);
    tzset();
    ESP_LOGI(kTag, "Setting up time zone... done");

    bool isTimeSynced = false;

    // Initialize NTP only if the device is connected to some external wifi
    // network
    if (mWifiManager.getMode() == WifiManager::Mode::Sta) {
        ESP_LOGI(kTag, "Initialize SNTP...");
        initializeSNTP();
        ESP_LOGI(kTag, "Initialize SNTP... done");

        ESP_LOGI(kTag, "Synchronizing time with NTP server...");
        int retry = 0;
        const int retry_count = 10;
        while (++retry < retry_count) {
            if (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
                isTimeSynced = true;
                break;
            }
            ESP_LOGI(kTag, "Waiting for system time to be set... (%d/%d)",
                     retry, retry_count);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(kTag, "Synchronizing time with NTP server... %s",
                 isTimeSynced ? "Done" : "Failed");
    }

    // If time is not synced with SNTP, try to sync time with RTC
    if (!isTimeSynced) {
        struct tm rtcTimeTm;
        if (mRtc.getTime(&rtcTimeTm)) {
            time_t rtcTime = timegmRtc(&rtcTimeTm);
            struct timeval tv = {.tv_sec = rtcTime, .tv_usec = 0};
            if (settimeofday(&tv, nullptr) == 0) {
                ESP_LOGI(kTag,
                         "System time set from RTC: %04d-%02d-%02d "
                         "%02d:%02d:%02d UTC",
                         rtcTimeTm.tm_year + 1900, rtcTimeTm.tm_mon + 1,
                         rtcTimeTm.tm_mday, rtcTimeTm.tm_hour, rtcTimeTm.tm_min,
                         rtcTimeTm.tm_sec);
                isTimeSynced = true;
            } else {
                ESP_LOGE(kTag, "Failed to set system time from RTC");
            }
        }
    }

    handleSleepMode();

    // Show current time after the current time is synced
    if (isTimeSynced && !isInSleepMode()) {
        startShowCurrentTimeTask();
        xTaskCreate(loopTask, "loopTask", 4096, this, 2, nullptr);
    }
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
    mTimeInfo = timeInfo;
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
    esp_sntp_set_time_sync_notification_cb(timeSyncNotificationCallback);
    esp_sntp_init();
}

void NixieClock::timeSyncNotificationCallback(struct timeval* tv) {
    if (!gRtcPtr) {
        return;
    }
    time_t now = tv->tv_sec;
    struct tm utcTime;
    gmtime_r(&now, &utcTime);
    ESP_LOGI(kTag, "Set time received from NTP to RTC");
    gRtcPtr->setTime(&utcTime);
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
        self->handleSleepMode();
        if (timeInfo.tm_sec == 0 && lastSecond != 0 && !self->isInSleepMode()) {
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
    struct tm nowTm;
    time(&now);
    localtime_r(&now, &nowTm);
    char buf[32];
    strftime(buf, sizeof(buf), "%H:%M:%S", &nowTm);
    ESP_LOGI(kTag, "Current time: %s", buf);
    LedInfo ledInfo = self->mLedController.getLedInfo();
    if (ledInfo.getState() != LedState::Off) {
        ledInfo.setState(LedState::On);
        self->mLedController.setTemporalState(ledInfo);
    }
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
    // handle 24 and 12 hour formats
    uint8_t hour = nowTm.tm_hour;
    if (self->mTimeInfo.getTimeFormat() == TimeFormat::Hour12) {
        hour = nowTm.tm_hour % 12;
        if (hour == 0) {
            hour = 12;
        }
    }
    int32_t nixieOutput[] = {hour / 10, hour % 10, nowTm.tm_min / 10,
                             nowTm.tm_min % 10};
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
    self->mLedController.clearTemporalState();
    self->mShowCurrentTimeTaskHandle = nullptr;
    vTaskDelete(nullptr);
}

void NixieClock::handleSleepMode() {
    bool currentSleepModeStatus = isInSleepMode();
    if (mLastSleepModeStatus != currentSleepModeStatus) {
        mLastSleepModeStatus = currentSleepModeStatus;
        if (mLastSleepModeStatus) {
            ESP_LOGI(kTag, "Entering sleep mode.");
            auto ledInfo = mLedController.getLedInfo();
            ledInfo.setState(LedState::Off);
            mLedController.setLedInfo(ledInfo);
        } else {
            ESP_LOGI(kTag, "Exiting sleep mode.");
            LedInfo ledInfo = ConfigStore::loadLedInfo().value_or(LedInfo());
            mLedController.setLedInfo(ledInfo);
        }
    }
}

time_t NixieClock::timegmRtc(struct tm* tm) {
    // Save current TZ
    char* oldTz = getenv("TZ");
    if (oldTz)
        oldTz = strdup(oldTz);   // copy for restore
    // Set TZ to UTC
    setenv("TZ", "UTC0", 1);
    tzset();
    time_t t = mktime(tm);   // mktime now treats tm as UTC
    // Restore previous TZ
    if (oldTz) {
        setenv("TZ", oldTz, 1);
        free(oldTz);
    } else {
        unsetenv("TZ");
    }
    tzset();
    return t;
}