#pragma once

#include <inttypes.h>

#include "LedInfo.h"

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
};