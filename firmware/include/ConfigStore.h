#pragma once

#include "LedInfo.h"
#include "SleepInfo.h"
#include "WifiInfo.h"

/**
 * @brief Class used for reading and writing config parameters
 */
class ConfigStore {
  public:
    /**
     * @brief Save led info
     *
     * @param ledInfo led info
     */
    static void SaveLedInfo(const LedInfo& ledInfo);

    /**
     * @brief Load led info
     *
     * @param ledInfo led info
     */
    static void LoadLedInfo(LedInfo& ledInfo);

    /**
     * @brief Save sleep info
     *
     * @param sleepInfo sleep info
     */
    static void SaveSleepInfo(const SleepInfo& sleepInfo);

    /**
     * @brief Load sleep info
     *
     * @param sleepInfo sleep info
     */
    static void LoadSleepInfo(SleepInfo& sleepInfo);

    /**
     * @brief Load wifi info
     *
     * @param wifiInfo wifi info
     */
    static void LoadWifiInfo(WifiInfo& sleepInfo);

    /**
     * @brief Save wifi info
     *
     * @param wifiInfo wifi info
     */
    static void SaveWifiInfo(const WifiInfo& wifiInfo);
};