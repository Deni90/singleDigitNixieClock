/******************************************************************************
 * File:    config_store.h
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Declarations for the ConfigStore class
 ******************************************************************************/

#ifndef config_store_h
#define config_store_h

#include <optional>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "led_info.h"
#include "sleep_info.h"
#include "time_info.h"
#include "wifi_info.h"

/**
 * @brief Class used for reading and writing config parameters
 */
class ConfigStore {
  public:
    /**
     * @brief Initialize the module
     */
    static void initialize();

    /**
     * @brief Load led info
     *
     * @param return LedInfo if the file is successfully read
     */
    static std::optional<LedInfo> loadLedInfo();

    /**
     * @brief Save led info
     *
     * @param ledInfo led info
     * @return True if the config file is successfully saved
     */
    static bool saveLedInfo(const LedInfo& ledInfo);

    /**
     * @brief Load sleep info
     *
     * @param return SleepInfo if the file is successfully read
     */
    static std::optional<SleepInfo> loadSleepInfo();

    /**
     * @brief Save sleep info
     *
     * @param sleepInfo sleep info
     * @return True if the config file is successfully saved
     */
    static bool saveSleepInfo(const SleepInfo& sleepInfo);

    /**
     * @brief Load wifi info
     *
     * @param return WifiInfo if the file is successfully read
     */
    static std::optional<WifiInfo> loadWifiInfo();

    /**
     * @brief Save wifi info
     *
     * @param wifiInfo wifi info
     * @return True if the config file is successfully saved
     */
    static bool saveWifiInfo(const WifiInfo& wifiInfo);

    /**
     * @brief Load time info
     *
     * @param return TimeInfo if the file is successfully read
     */
    static std::optional<TimeInfo> loadTimeInfo();

    /**
     * @brief Save wifi info
     *
     * @param timeInfo time info
     * @return True if the config file is successfully saved
     */
    static bool saveTimeInfo(const TimeInfo& timeInfo);

  private:
    static SemaphoreHandle_t mMutex;
};

#endif   // config_store_h