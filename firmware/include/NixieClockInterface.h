#pragma once

#include "ClockFace.h"
#include "ClockInterface.h"
#include "LedController.h"
#include <RtcDS3231.h>
#include <Wire.h>

/**
 * @brief Represents an implementation of ClockInterface
 */
class NixieClockInterface : public ClockInterface {
  private:
    LedController& ledController;
    RtcDS3231<TwoWire>& rtc;
    ClockFace& clock;

  public:
    /**
     * @brief Construct a new Nixie Clock Interface object
     *
     * @param ledController reference to led controller object
     * @param rtc reference to real time clock object
     * @param clock reference tto clock face object
     */
    NixieClockInterface(LedController& ledController, RtcDS3231<TwoWire>& rtc,
                        ClockFace& clock);

    /**
     * @brief Return led info
     *
     * @return LedInfo object containing led info
     */
    LedInfo OnGetBacklightData() const;

    /**
     * @brief Set backlight state and save change in the config
     *
     * @param state LedState value
     * @return true successfully set led state
     * @return false failed to set led state. Bad led state provided.
     */
    bool OnSetBacklightState(uint8_t state);

    /**
     * @brief Set backlight color and save changes in the config
     *
     * @param r red value
     * @param g green value
     * @param b blue value
     */
    void OnSetBacklightColor(uint8_t r, uint8_t g, uint8_t b);

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
    void OnSetCurrentTime(uint16_t year, uint8_t month, uint8_t dayOfMonth,
                          uint8_t hour, uint8_t minute, uint8_t second);
};