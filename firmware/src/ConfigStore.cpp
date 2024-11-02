#include "ConfigStore.h"
#include <EEPROM.h>
#include <inttypes.h>

namespace {
constexpr uint16_t EEPROM_SIZE = 512;
constexpr uint16_t EEPROM_LED_INFO_START_ADDR = 0;
constexpr uint16_t EEPROM_SLEEP_INFO_START_ADDR = 100;
}   // namespace

void ConfigStore::SaveLedInfo(const LedInfo& ledInfo) {
    EEPROM.begin(EEPROM_SIZE);   // Initialize EEPROM
    uint16_t eepromAddr = EEPROM_LED_INFO_START_ADDR;
    EEPROM.write(eepromAddr, ledInfo.GetR());
    eepromAddr++;
    EEPROM.write(eepromAddr, ledInfo.GetG());
    eepromAddr++;
    EEPROM.write(eepromAddr, ledInfo.GetB());
    eepromAddr++;
    EEPROM.write(eepromAddr, static_cast<uint8_t>(ledInfo.GetState()));
    EEPROM.commit();   // Store data to EEPROM
}

void ConfigStore::LoadLedInfo(LedInfo& ledInfo) {
    EEPROM.begin(EEPROM_SIZE);   // Initialize EEPROM
    uint16_t eepromAddr = EEPROM_LED_INFO_START_ADDR;
    uint8_t r = EEPROM.read(eepromAddr);
    eepromAddr++;
    uint8_t g = EEPROM.read(eepromAddr);
    eepromAddr++;
    uint8_t b = EEPROM.read(eepromAddr);
    eepromAddr++;
    uint8_t state = EEPROM.read(eepromAddr);

    ledInfo.SetColor(r, g, b);
    ledInfo.SetState(static_cast<LedState>(state));
}

void ConfigStore::SaveSleepInfo(const SleepInfo& sleepInfo) {
    EEPROM.begin(EEPROM_SIZE);   // Initialize EEPROM
    uint16_t eepromAddr = EEPROM_SLEEP_INFO_START_ADDR;
    EEPROM.write(eepromAddr, sleepInfo.GetSleepBefore());
    eepromAddr++;
    EEPROM.write(eepromAddr, sleepInfo.GetSleepAfter());
    EEPROM.commit();   // Store data to EEPROM
}

void ConfigStore::LoadSleepInfo(SleepInfo& sleepInfo) {
    EEPROM.begin(EEPROM_SIZE);   // Initialize EEPROM
    uint16_t eepromAddr = EEPROM_SLEEP_INFO_START_ADDR;
    uint8_t sb = EEPROM.read(eepromAddr);
    eepromAddr++;
    uint8_t sa = EEPROM.read(eepromAddr);

    sleepInfo.SetSleepBefore(sb);
    sleepInfo.SetSleepAfter(sa);
}
