#pragma once

#include <RtcDS3231.h>
#include <inttypes.h>

#include "In14NixieTube.h"
#include "LedController.h"

/**
 * @brief Represents the actual implementation of the clock
 *
 * This clas is responsible for showing the current time by handling the RGB led
 * and the nixie tube.
 *
 */
class ClockFace {
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
    RtcDateTime time;
    uint8_t repeatNumber;

  public:
    /**
     * @brief Construct a new Clock Face object
     *
     * @param ledController led controler
     * @param nixieTube nixie tube
     */
    ClockFace(LedController& ledController, In14NixieTube& nixieTube);

    void Handle(uint32_t& tick);
    /**
     * @brief  Show time on the nixie tube
     *
     * @param now The current time
     * @param repeat The number of times the time is shown on the nixie tube
     */
    void ShowTime(RtcDateTime now, uint8_t repeat = 1);
};