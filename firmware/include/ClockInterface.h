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
    virtual LedInfo OnGetBacklightData() const = 0;

    /**
     * @brief Set backlight state and save change in the config
     *
     * @param state LedState value
     * @return true successfully set led state
     * @return false failed to set led state. Bad led state provided.
     */
    virtual bool OnSetBacklightState(uint8_t state) = 0;

    /**
     * @brief Set backlight color and save changes in the config
     *
     * @param r red value
     * @param g green value
     * @param b blue value
     */
    virtual void OnSetBacklightColor(uint8_t r, uint8_t g, uint8_t b) = 0;

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