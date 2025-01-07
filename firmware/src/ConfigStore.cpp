#include "ConfigStore.h"
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <inttypes.h>

namespace {
constexpr const char* LED_INFO_FILE = "/config/led_info.json";
constexpr const char* SLEEP_INFO_FILE = "/config/sleep_info.json";
constexpr const char* TIME_INFO_FILE = "/config/time_info.json";
constexpr const char* WIFI_INFO_FILE = "/config/wifi_info.json";
}   // namespace

void ConfigStore::SaveLedInfo(const LedInfo& ledInfo) {
    String messageBuffer;
    serializeJson(ledInfo.ToJson(), messageBuffer);
    File file = LittleFS.open(LED_INFO_FILE, "w");
    file.print(messageBuffer);
    file.close();
}

void ConfigStore::LoadLedInfo(LedInfo& ledInfo) {
    File file = LittleFS.open(LED_INFO_FILE, "r");
    if (!file) {
        Serial.printf("Failed to open %s file.\n", LED_INFO_FILE);
        return;
    }
    JsonDocument doc;
    if (deserializeJson(doc, file))
        Serial.printf("Failed to read %s file.\n", LED_INFO_FILE);
    file.close();
    ledInfo.SetColor(doc["R"], doc["G"], doc["B"]);
    uint8_t state = doc["state"];
    ledInfo.SetState(static_cast<LedState>(state));
}

void ConfigStore::SaveSleepInfo(const SleepInfo& sleepInfo) {
    String messageBuffer;
    serializeJson(sleepInfo.ToJson(), messageBuffer);
    File file = LittleFS.open(SLEEP_INFO_FILE, "w");
    file.print(messageBuffer);
    file.close();
}

void ConfigStore::LoadSleepInfo(SleepInfo& sleepInfo) {
    File file = LittleFS.open(SLEEP_INFO_FILE, "r");
    if (!file) {
        Serial.printf("Failed to open %s file.\n", SLEEP_INFO_FILE);
        return;
    }
    JsonDocument doc;
    if (deserializeJson(doc, file))
        Serial.printf("Failed to read %s file.\n", SLEEP_INFO_FILE);
    file.close();
    sleepInfo.SetSleepBefore(doc["sleep_before"]);
    sleepInfo.SetSleepAfter(doc["sleep_after"]);
}

void ConfigStore::LoadWifiInfo(WifiInfo& wifiInfo) {
    File file = LittleFS.open(WIFI_INFO_FILE, "r");
    if (!file) {
        Serial.printf("Failed to open %s file.\n", WIFI_INFO_FILE);
        return;
    }
    JsonDocument doc;
    if (deserializeJson(doc, file))
        Serial.printf("Failed to read %s file.\n", WIFI_INFO_FILE);
    file.close();
    wifiInfo.SetSSID(doc["SSID"]);
    wifiInfo.SetPassword(doc["password"]);
}

void ConfigStore::SaveWifiInfo(const WifiInfo& wifiInfo) {
    String messageBuffer;
    serializeJson(wifiInfo.ToJson(), messageBuffer);
    File file = LittleFS.open(WIFI_INFO_FILE, "w");
    file.print(messageBuffer);
    file.close();
}

void ConfigStore::LoadTimeInfo(TimeInfo& timeInfo) {
    File file = LittleFS.open(TIME_INFO_FILE, "r");
    if (!file) {
        Serial.printf("Failed to open %s file.\n", TIME_INFO_FILE);
        return;
    }
    JsonDocument doc;
    if (deserializeJson(doc, file))
        Serial.printf("Failed to read %s file.\n", TIME_INFO_FILE);
    file.close();
    timeInfo.SetOffset(doc["offset"]);
}

void ConfigStore::SaveTimeInfo(const TimeInfo& timeInfo) {
    String messageBuffer;
    serializeJson(timeInfo.ToJson(), messageBuffer);
    File file = LittleFS.open(TIME_INFO_FILE, "w");
    file.print(messageBuffer);
    file.close();
}