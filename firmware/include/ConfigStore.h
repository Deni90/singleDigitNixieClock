#pragma once

#include "LedInfo.h"
#include "SleepInfo.h"

/**
 * @brief Class used for reading and writing congfig parameters from EEPROM
 */
class ConfigStore {
  public:
    /**
     * @brief Save led info to EEPROM
     *
     * @param ledInfo led info
     */
    static void SaveLedConfiguration(const LedInfo& ledInfo);

    /**
     * @brief Load led info from EEPROM
     *
     * @param ledInfo led info
     */
    static void LoadLedConfiguration(LedInfo& ledInfo);

    /**
     * @brief Save sleep info to EEPROM
     *
     * @param sleepInfo sleep info
     */
    static void SaveSleepInfo(const SleepInfo& sleepInfo);

    /**
     * @brief Load sleep info from EEPROM
     *
     * @param sleepInfo sleep info
     */
    static void LoadSleepInfo(SleepInfo& sleepInfo);
};