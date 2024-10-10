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
    LedInfo OnGetLedInfo() const;

    /**
     * @brief Set led info and save changes in config store
     *
     * @param ledInfo led info
     */
    void OnSetLedInfo(const LedInfo& ledInfo);

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