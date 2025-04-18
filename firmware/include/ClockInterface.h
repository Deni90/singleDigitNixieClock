#pragma once

#include <inttypes.h>

#include "LedInfo.h"
#include "SleepInfo.h"
#include "TimeInfo.h"
#include "WifiInfo.h"

/**
 * @brief Definition of clock interface
 *
 */
class ClockInterface {
  public:
    /**
     * @brief Default destructor
     *
     */
    virtual ~ClockInterface() = default;

    /**
     * @brief Return led info
     *
     * @return LedInfo object containing led info
     */
    virtual LedInfo OnGetLedInfo() const = 0;

    /**
     * @brief Set led info and save changes in config store
     *
     * @param ledInfo led info
     */
    virtual void OnSetLedInfo(const LedInfo& ledInfo) = 0;

    /**
     * @brief Set current date and time to RTC
     *
     * @param year year
     * @param month  month
     * @param dayOfMonth day
     * @param hour hour
     * @param minute minute
     * @param second second
     */
    virtual void OnSetCurrentTime(uint16_t year, uint8_t month,
                                  uint8_t dayOfMonth, uint8_t hour,
                                  uint8_t minute, uint8_t second) = 0;

    /**
     * @brief Return sleep info
     *
     * @return SleepInfo object containing sleep info
     */
    virtual SleepInfo OnGetSleepInfo() const = 0;

    /**
     * @brief Set sleep info and save changes in config store
     *
     * @param sleepInfo sleep info
     */
    virtual void OnSetSleepInfo(const SleepInfo& sleepInfo) = 0;

    /**
     * @brief Return wifi info
     *
     * @return WifiInfo object containing wifi info
     */
    virtual WifiInfo OnGetWifiInfo() const = 0;

    /**
     * @brief Set wifi info and save changes in config store
     *
     * @param wifiInfo wifi info
     */
    virtual void OnSetWifiInfo(const WifiInfo& wifiInfo) = 0;

    /**
     * @brief Return time info
     *
     * @return TimeInfo object containing time info
     */
    virtual TimeInfo OnGetTimeInfo() const = 0;

    /**
     * @brief Set time info and save changes in config store
     *
     * @param timeInfo time info
     */
    virtual void OnSetTimeInfo(const TimeInfo& timeInfo) = 0;
};