#pragma once

#include <RtcDS3231.h>
#include <Wire.h>
#include <inttypes.h>

#include "ClockInterface.h"
#include "In14NixieTube.h"
#include "LedController.h"
#include "SleepInfo.h"

/**
 * @brief Represents the actual implementation of the clock
 *
 * This clas is responsible for showing the current time by handling the RGB led
 * and the nixie tube.
 *
 */
class NixieClock : public ClockInterface {
  private:
    enum class AnimationStates {
        IDLE = 0,
        INIT,
        INTRO,
        SHOW_TIME,
        PAUSE,
        CLEANUP
    };
    AnimationStates animationState;
    LedController& ledController;
    In14NixieTube& nixieTube;
    RtcDS3231<TwoWire>& rtc;
    RtcDateTime time;
    uint8_t repeatNumber;
    SleepInfo sleepInfo;

    bool IsInSleepMode();

  public:
    /**
     * @brief Construct a new NixieClock object
     *
     * @param ledController led controler
     * @param nixieTube nixie tube
     */
    NixieClock(LedController& ledController, In14NixieTube& nixieTube,
               RtcDS3231<TwoWire>& rtc);

    /**
     * @brief Initialize module
     *
     * @param sleep_info Sleep info
     */
    void Initialize(const SleepInfo& sleepInfo);

    void Handle(uint32_t& tick);
    /**
     * @brief  Show time on the nixie tube
     *
     * @param now The current time
     * @param repeat The number of times the time is shown on the nixie tube
     */
    void ShowTime(RtcDateTime now, uint8_t repeat = 1);

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

    /**
     * @brief Return sleep info
     *
     * @return SleepInfo object containing sleep info
     */
    SleepInfo OnGetSleepInfo() const;

    /**
     * @brief Set sleep info and save changes in config store
     *
     * @param sleepInfo sleep info
     */
    void OnSetSleepInfo(const SleepInfo& sleepInfo);
};