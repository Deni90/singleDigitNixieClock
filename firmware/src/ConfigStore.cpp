#include "ConfigStore.h"
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <inttypes.h>

namespace {
constexpr const char* LED_INFO_FILE = "/config/led_info.json";
constexpr const char* SLEEP_INFO_FILE = "/config/sleep_info.json";
}   // namespace

void ConfigStore::SaveLedInfo(const LedInfo& ledInfo) {
    JsonDocument doc;
    String messageBuffer;
    doc["state"] = static_cast<uint8_t>(ledInfo.GetState());
    doc["R"] = ledInfo.GetR();
    doc["G"] = ledInfo.GetG();
    doc["B"] = ledInfo.GetB();
    serializeJson(doc, messageBuffer);
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
    JsonDocument doc;
    String messageBuffer;
    doc["sleep_before"] = sleepInfo.GetSleepBefore();
    doc["sleep_after"] = sleepInfo.GetSleepAfter();
    serializeJson(doc, messageBuffer);
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