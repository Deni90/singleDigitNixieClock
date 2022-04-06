#include <inttypes.h>
#include <EEPROM.h>
#include "ConfigStore.h"

namespace
{
    constexpr uint16_t EEPROM_SIZE = 512;
    constexpr uint16_t EEPROM_START_ADDR = 0;
}

void ConfigStore::SaveLedConfiguration(const LedInfo& ledInfo)
{
    EEPROM.begin(EEPROM_SIZE);  //Initialize EEPROM
    uint16_t eepromAddr = EEPROM_START_ADDR;
    EEPROM.write(eepromAddr, ledInfo.GetR());
    eepromAddr++;
    EEPROM.write(eepromAddr, ledInfo.GetG());
    eepromAddr++;
    EEPROM.write(eepromAddr, ledInfo.GetB());
    eepromAddr++;
    EEPROM.write(eepromAddr, ledInfo.GetA());
    eepromAddr++;
    EEPROM.write(eepromAddr, static_cast<uint8_t>(ledInfo.GetState()));
    eepromAddr++;
    EEPROM.commit();    //Store data to EEPROM
}

void ConfigStore::LoadLedConfiguration(LedInfo& ledInfo)
{
    EEPROM.begin(EEPROM_SIZE);  //Initialize EEPROM
    uint16_t eepromAddr = EEPROM_START_ADDR;
    uint8_t r = EEPROM.read(eepromAddr);
    eepromAddr++;
    uint8_t g = EEPROM.read(eepromAddr);
    eepromAddr++;
    uint8_t b = EEPROM.read(eepromAddr);
    eepromAddr++;
    uint8_t a = EEPROM.read(eepromAddr);
    eepromAddr++;
    uint8_t state = EEPROM.read(eepromAddr);

    ledInfo.SetColor(r, g, b, a);
    ledInfo.SetState(static_cast<LedState>(state));
}