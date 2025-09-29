/******************************************************************************
 * File:    wifi_manager.cpp
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Implements WifiManager data class
 ******************************************************************************/

#include "wifi_manager.h"

#include <cstring>

#include "esp_log.h"
#include "mbedtls/base64.h"
#include "nvs_flash.h"   //non volatile storage

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char* kTag = "wifi_manager";
static constexpr int kMaxConnectionRetry = 10;

WifiManager::WifiManager()
    : mMode(WifiManager::Mode::None), mWifiEventGroup(xEventGroupCreate()) {}

void WifiManager::initialize() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

void WifiManager::eventHandlerStatic(void* arg, esp_event_base_t eventBase,
                                     int32_t eventId, void* eventData) {
    static_cast<WifiManager*>(arg)->eventHandler(eventBase, eventId, eventData);
}

void WifiManager::eventHandler(esp_event_base_t eventBase, int32_t eventId,
                               void* eventData) {
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
            xEventGroupSetBits(mWifiEventGroup, WIFI_FAIL_BIT);
        }

    } else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = static_cast<ip_event_got_ip_t*>(eventData);
        ESP_LOGI(kTag, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        retryCount = 0;
        xEventGroupSetBits(mWifiEventGroup, WIFI_CONNECTED_BIT);
    }
}

bool WifiManager::connectSta(const WifiInfo& wifiInfo) {
    esp_netif_t* netif = esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(
        esp_netif_set_hostname(netif, wifiInfo.getHostname().c_str()));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiManager::eventHandlerStatic, this,
        nullptr));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &WifiManager::eventHandlerStatic, this,
        nullptr));

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
    switch (wifiInfo.getAuthType()) {
    case WifiAuthType::Open:
        wifiConfig.sta.threshold.authmode = WIFI_AUTH_OPEN;
        break;
    case WifiAuthType::WPA2:
        wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        break;
    case WifiAuthType::WPA3:
        wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA3_PSK;
        break;
    }
    wifiConfig.sta.pmf_cfg.capable = true;
    wifiConfig.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits =
        xEventGroupWaitBits(mWifiEventGroup, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                            pdFALSE, pdFALSE, pdMS_TO_TICKS(10000));

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(kTag, "Connected to STA: %s", wifiInfo.getSSID().c_str());
        mMode = WifiManager::Mode::Sta;
        return true;
    }

    ESP_LOGW(kTag, "Failed to connect to STA.");
    return false;
}

void WifiManager::startAp(const WifiInfo& wifiInfo) {
    esp_netif_t* netif = esp_netif_create_default_wifi_ap();
    ESP_ERROR_CHECK(
        esp_netif_set_hostname(netif, wifiInfo.getHostname().c_str()));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifiConfig = {};
    std::strncpy(reinterpret_cast<char*>(wifiConfig.ap.ssid),
                 wifiInfo.getSSID().c_str(), sizeof(wifiConfig.ap.ssid));
    wifiConfig.ap.ssid_len = wifiInfo.getSSID().length();
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
    mMode = WifiManager::Mode::Ap;
}

WifiManager::Mode WifiManager::getMode() const { return mMode; }