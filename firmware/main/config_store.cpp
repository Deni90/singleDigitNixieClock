/******************************************************************************
 * File:    config_store.cpp
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Implements ConfigStore class for persistent configuration
 *          management.
 ******************************************************************************/

#include "config_store.h"

#include <filesystem>
#include <fstream>
#include <inttypes.h>
#include <iostream>
#include <sstream>

#include "cJSON.h"
#include "esp_littlefs.h"
#include "esp_log.h"

static const char* kTag = "config_store";
static constexpr const char* kLedInfoFile = "/littlefs/config/led_info.json";
static constexpr const char* kSleepInfoFile =
    "/littlefs/config/sleep_info.json";
static constexpr const char* kTimeInfoFile = "/littlefs/config/time_info.json";
static constexpr const char* kWifiInfoFile = "/littlefs/config/wifi_info.json";

SemaphoreHandle_t ConfigStore::mMutex = nullptr;
bool ConfigStore::mIsInitialized = false;

void ConfigStore::initialize() {
    if (!mIsInitialized) {
        setupLittlefs();
        if (mMutex == nullptr) {
            mMutex = xSemaphoreCreateMutex();
        }
        mIsInitialized = true;
    }
}

std::optional<LedInfo> ConfigStore::loadLedInfo() {
    if (!mIsInitialized) {
        ESP_LOGE(kTag,
                 "Module not initialized, intitialize it before using it.");
        return std::nullopt;
    }
    // check if file exists on kLedInfoFile path
    if (!std::filesystem::exists(kLedInfoFile)) {
        return std::nullopt;
    }
    // read and parse JSON file
    std::stringstream buffer;
    if (mMutex && xSemaphoreTake(mMutex, portMAX_DELAY) == pdTRUE) {
        std::ifstream ledInfoFile(kLedInfoFile);
        buffer << ledInfoFile.rdbuf();
        xSemaphoreGive(mMutex);
    } else {
        return std::nullopt;
    }
    cJSON* json = cJSON_Parse(buffer.str().c_str());
    // check are all fields present in the config
    if (!cJSON_GetObjectItemCaseSensitive(json, "R")) {
        ESP_LOGW(kTag, "'R' is not found in config");
        cJSON_Delete(json);
        return std::nullopt;
    }
    if (!cJSON_GetObjectItemCaseSensitive(json, "G")) {
        ESP_LOGW(kTag, "'G' is not found in config");
        cJSON_Delete(json);
        return std::nullopt;
    }
    if (!cJSON_GetObjectItemCaseSensitive(json, "B")) {
        ESP_LOGW(kTag, "'B' is not found in config");
        cJSON_Delete(json);
        return std::nullopt;
    }
    // populate LedInfo object
    LedInfo ledInfo;
    ledInfo.setColor(cJSON_GetObjectItemCaseSensitive(json, "R")->valueint,
                     cJSON_GetObjectItemCaseSensitive(json, "G")->valueint,
                     cJSON_GetObjectItemCaseSensitive(json, "B")->valueint);
    std::string state =
        cJSON_GetObjectItemCaseSensitive(json, "state")->valuestring;
    if (state == "on") {
        ledInfo.setState(LedState::On);
    } else if (state == "off") {
        ledInfo.setState(LedState::Off);
    } else if (state == "fade") {
        ledInfo.setState(LedState::Fade);
    } else if (state == "pulse") {
        ledInfo.setState(LedState::Pulse);
    }
    cJSON_Delete(json);
    return ledInfo;
}

bool ConfigStore::saveLedInfo(const LedInfo& ledInfo) {
    if (!mIsInitialized) {
        ESP_LOGE(kTag,
                 "Module not initialized, intitialize it before using it.");
        return false;
    }
    bool ret = true;
    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "R", cJSON_CreateNumber(ledInfo.getRed()));
    cJSON_AddItemToObject(json, "G", cJSON_CreateNumber(ledInfo.getGreen()));
    cJSON_AddItemToObject(json, "B", cJSON_CreateNumber(ledInfo.getBlue()));
    cJSON_AddItemToObject(
        json, "state",
        cJSON_CreateString(ledStateToString(ledInfo.getState())));
    char* jsonString = cJSON_Print(json);
    if (jsonString != nullptr) {
        if (mMutex && xSemaphoreTake(mMutex, portMAX_DELAY) == pdTRUE) {
            std::ofstream ledInfoFile(kLedInfoFile);
            ledInfoFile << jsonString;
            xSemaphoreGive(mMutex);
        } else {
            ret = false;
        }
        cJSON_free(jsonString);
    }
    cJSON_Delete(json);
    return ret;
}

std::optional<SleepInfo> ConfigStore::loadSleepInfo() {
    if (!mIsInitialized) {
        ESP_LOGE(kTag,
                 "Module not initialized, intitialize it before using it.");
        return std::nullopt;
    }
    // check if file exists on kSleepInfoFile path
    if (!std::filesystem::exists(kSleepInfoFile)) {
        return std::nullopt;
    }
    // read and parse JSON file
    std::stringstream buffer;
    if (mMutex && xSemaphoreTake(mMutex, portMAX_DELAY) == pdTRUE) {
        std::ifstream sleepInfoFile(kSleepInfoFile);
        buffer << sleepInfoFile.rdbuf();
        xSemaphoreGive(mMutex);
    } else {
        return std::nullopt;
    }
    cJSON* json = cJSON_Parse(buffer.str().c_str());
    // check are all fields present in the config
    if (!cJSON_GetObjectItemCaseSensitive(json, "sleep_before")) {
        ESP_LOGW(kTag, "'sleep_before' is not found in config");
        cJSON_Delete(json);
        return std::nullopt;
    }
    if (!cJSON_GetObjectItemCaseSensitive(json, "sleep_after")) {
        ESP_LOGW(kTag, "'sleep_after' is not found in config");
        cJSON_Delete(json);
        return std::nullopt;
    }
    // populate SleepInfo object
    SleepInfo sleepInfo;
    sleepInfo.setSleepBefore(
        cJSON_GetObjectItemCaseSensitive(json, "sleep_before")->valueint);
    sleepInfo.setSleepAfter(
        cJSON_GetObjectItemCaseSensitive(json, "sleep_after")->valueint);
    cJSON_Delete(json);
    return sleepInfo;
}

bool ConfigStore::saveSleepInfo(const SleepInfo& sleepInfo) {
    if (!mIsInitialized) {
        ESP_LOGE(kTag,
                 "Module not initialized, intitialize it before using it.");
        return false;
    }
    bool ret = true;
    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "sleep_before",
                          cJSON_CreateNumber(sleepInfo.getSleepBefore()));
    cJSON_AddItemToObject(json, "sleep_after",
                          cJSON_CreateNumber(sleepInfo.getSleepAfter()));
    char* jsonString = cJSON_Print(json);
    if (jsonString != nullptr) {
        if (mMutex && xSemaphoreTake(mMutex, portMAX_DELAY) == pdTRUE) {
            std::ofstream sleepInfoFile(kSleepInfoFile);
            sleepInfoFile << jsonString;
            xSemaphoreGive(mMutex);
        } else {
            ret = false;
        }
        cJSON_free(jsonString);
    }
    cJSON_Delete(json);
    return ret;
}

std::optional<WifiInfo> ConfigStore::loadWifiInfo() {
    if (!mIsInitialized) {
        ESP_LOGE(kTag,
                 "Module not initialized, intitialize it before using it.");
        return std::nullopt;
    }
    // check if file exists on kWifiInfoFile path
    if (!std::filesystem::exists(kWifiInfoFile)) {
        return std::nullopt;
    }
    // read and parse JSON file
    std::stringstream buffer;
    if (mMutex && xSemaphoreTake(mMutex, portMAX_DELAY) == pdTRUE) {
        std::ifstream wifiInfoFile(kWifiInfoFile);
        buffer << wifiInfoFile.rdbuf();
        xSemaphoreGive(mMutex);
    } else {
        return std::nullopt;
    }
    cJSON* json = cJSON_Parse(buffer.str().c_str());
    // check are all fields present in the config
    if (!cJSON_GetObjectItemCaseSensitive(json, "hostname")) {
        ESP_LOGW(kTag, "'hostname' is not found in config");
        cJSON_Delete(json);
        return std::nullopt;
    }
    if (!cJSON_GetObjectItemCaseSensitive(json, "SSID")) {
        ESP_LOGW(kTag, "'SSID' is not found in config");
        cJSON_Delete(json);
        return std::nullopt;
    }
    if (!cJSON_GetObjectItemCaseSensitive(json, "auth_type")) {
        ESP_LOGW(kTag, "'auth_type' is not found in config");
        cJSON_Delete(json);
        return std::nullopt;
    }
    if (!cJSON_GetObjectItemCaseSensitive(json, "password")) {
        ESP_LOGW(kTag, "'password' is not found in config");
        cJSON_Delete(json);
        return std::nullopt;
    }
    // populate WifiInfo object
    WifiInfo wifiInfo;
    wifiInfo.setHostname(
        cJSON_GetObjectItemCaseSensitive(json, "hostname")->valuestring);
    wifiInfo.setSSID(
        cJSON_GetObjectItemCaseSensitive(json, "SSID")->valuestring);
    std::string authType =
        cJSON_GetObjectItemCaseSensitive(json, "auth_type")->valuestring;
    if (authType == "open") {
        wifiInfo.setAuthType(WifiAuthType::Open);
    } else if (authType == "wpa2") {
        wifiInfo.setAuthType(WifiAuthType::WPA2);
    } else if (authType == "wpa3") {
        wifiInfo.setAuthType(WifiAuthType::WPA3);
    } else {
        ESP_LOGW(kTag, "Invalid 'auth_format'");
        cJSON_Delete(json);
        return std::nullopt;
    }
    wifiInfo.setPassword(
        cJSON_GetObjectItemCaseSensitive(json, "password")->valuestring);
    cJSON_Delete(json);
    return wifiInfo;
}

bool ConfigStore::saveWifiInfo(const WifiInfo& wifiInfo) {
    if (!mIsInitialized) {
        ESP_LOGE(kTag,
                 "Module not initialized, intitialize it before using it.");
        return false;
    }
    bool ret = true;
    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "hostname",
                          cJSON_CreateString(wifiInfo.getHostname().c_str()));
    cJSON_AddItemToObject(json, "SSID",
                          cJSON_CreateString(wifiInfo.getSSID().c_str()));
    cJSON_AddItemToObject(
        json, "auth_type",
        cJSON_CreateString(wifiAuthTypeToString(wifiInfo.getAuthType())));
    cJSON_AddItemToObject(json, "password",
                          cJSON_CreateString(wifiInfo.getPassword().c_str()));
    char* jsonString = cJSON_Print(json);
    if (jsonString != nullptr) {
        if (mMutex && xSemaphoreTake(mMutex, portMAX_DELAY) == pdTRUE) {
            std::ofstream wifiInfoFile(kWifiInfoFile);
            wifiInfoFile << jsonString;
            xSemaphoreGive(mMutex);
        } else {
            ret = false;
        }
        cJSON_free(jsonString);
    }
    cJSON_Delete(json);
    return ret;
}

std::optional<TimeInfo> ConfigStore::loadTimeInfo() {
    if (!mIsInitialized) {
        ESP_LOGE(kTag,
                 "Module not initialized, intitialize it before using it.");
        return std::nullopt;
    }
    // check if file exists on kTimeInfoFile path
    if (!std::filesystem::exists(kTimeInfoFile)) {
        return std::nullopt;
    }
    // read and parse JSON file
    std::stringstream buffer;
    if (mMutex && xSemaphoreTake(mMutex, portMAX_DELAY) == pdTRUE) {
        std::ifstream timeInfoFile(kTimeInfoFile);
        buffer << timeInfoFile.rdbuf();
        xSemaphoreGive(mMutex);
    } else {
        return std::nullopt;
    }
    cJSON* json = cJSON_Parse(buffer.str().c_str());
    // check are all fields present in the config
    if (!cJSON_GetObjectItemCaseSensitive(json, "tz_zone")) {
        ESP_LOGW(kTag, "'tz_zone' is not found in config");
        cJSON_Delete(json);
        return std::nullopt;
    }
    if (!cJSON_GetObjectItemCaseSensitive(json, "tz_offset")) {
        ESP_LOGW(kTag, "'tz_offset' is not found in config");
        cJSON_Delete(json);
        return std::nullopt;
    }
    if (!cJSON_GetObjectItemCaseSensitive(json, "time_format")) {
        ESP_LOGW(kTag, "'time_format' is not found in config");
        cJSON_Delete(json);
        return std::nullopt;
    }
    // populate TimeInfo object
    TimeInfo timeInfo;
    timeInfo.setTzZone(
        cJSON_GetObjectItemCaseSensitive(json, "tz_zone")->valuestring);
    timeInfo.setTzOffset(
        cJSON_GetObjectItemCaseSensitive(json, "tz_offset")->valuestring);
    std::string timeFormat =
        cJSON_GetObjectItemCaseSensitive(json, "time_format")->valuestring;
    if (timeFormat == "24h") {
        timeInfo.setTimeFormat(TimeFormat::Hour24);
    } else if (timeFormat == "12h") {
        timeInfo.setTimeFormat(TimeFormat::Hour12);
    } else {
        ESP_LOGW(kTag, "Invalid 'time_format'");
        cJSON_Delete(json);
        return std::nullopt;
    }
    cJSON_Delete(json);
    return timeInfo;
}

bool ConfigStore::saveTimeInfo(const TimeInfo& timeInfo) {
    if (!mIsInitialized) {
        ESP_LOGE(kTag,
                 "Module not initialized, intitialize it before using it.");
        return false;
    }
    bool ret = true;
    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "tz_zone",
                          cJSON_CreateString(timeInfo.getTzZone().c_str()));
    cJSON_AddItemToObject(json, "tz_offset",
                          cJSON_CreateString(timeInfo.getTzOffset().c_str()));
    cJSON_AddItemToObject(
        json, "time_format",
        cJSON_CreateString(timeFormatToString(timeInfo.getTimeFormat())));
    char* jsonString = cJSON_Print(json);
    if (jsonString != nullptr) {
        if (mMutex && xSemaphoreTake(mMutex, portMAX_DELAY) == pdTRUE) {
            std::ofstream timeInfoFile(kTimeInfoFile);
            timeInfoFile << jsonString;
            xSemaphoreGive(mMutex);
        } else {
            ret = false;
        }
        cJSON_free(jsonString);
    }
    cJSON_Delete(json);
    return ret;
}

void ConfigStore::setupLittlefs() {
    esp_vfs_littlefs_conf_t conf = {};
    conf.base_path = "/littlefs";
    conf.partition_label = "littlefs";
    conf.format_if_mount_failed = true;
    conf.dont_mount = false;

    // Use settings defined above to initialize and mount LittleFS filesystem.
    // Note: esp_vfs_littlefs_register is an all-in-one convenience function.
    ESP_ERROR_CHECK(esp_vfs_littlefs_register(&conf));
}