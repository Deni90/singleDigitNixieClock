/******************************************************************************
 * File:    clock_iface.h
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Declaration of Clock interface
 ******************************************************************************/

#ifndef clock_iface_h
#define clock_iface_h

#include <inttypes.h>
#include <optional>

#include "led_info.h"
#include "sleep_info.h"
#include "time_info.h"
#include "wifi_info.h"

class IClock {
  public:
    /**
     * @brief Default destructor
     *
     */
    virtual ~IClock() = default;

    /**
     * @brief Return led info
     *
     * @return LedInfo object containing led info
     */
    virtual std::optional<LedInfo> onGetLedInfo() const = 0;

    /**
     * @brief Set led info and save changes in config store
     *
     * @param ledInfo led info
     */
    virtual void onSetLedInfo(const LedInfo& ledInfo) = 0;

    /**
     * @brief Return sleep info
     *
     * @return SleepInfo object containing sleep info
     */
    virtual std::optional<SleepInfo> onGetSleepInfo() const = 0;

    /**
     * @brief Set sleep info and save changes in config store
     *
     * @param sleepInfo sleep info
     */
    virtual void onSetSleepInfo(const SleepInfo& sleepInfo) = 0;

    /**
     * @brief Return wifi info
     *
     * @return WifiInfo object containing wifi info
     */
    virtual std::optional<WifiInfo> onGetWifiInfo() const = 0;

    /**
     * @brief Set wifi info and save changes in config store
     *
     * @param wifiInfo wifi info
     */
    virtual void onSetWifiInfo(const WifiInfo& wifiInfo) = 0;

    /**
     * @brief Return time info
     *
     * @return TimeInfo object containing time info
     */
    virtual std::optional<TimeInfo> onGetTimeInfo() const = 0;

    /**
     * @brief Set time info and save changes in config store
     *
     * @param timeInfo time info
     */
    virtual void onSetTimeInfo(const TimeInfo& timeInfo) = 0;
};

#endif   // clock_iface_h