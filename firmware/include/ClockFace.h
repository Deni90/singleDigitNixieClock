#pragma once

#include <RtcDS3231.h>
#include <inttypes.h>

#include "In14NixieTube.h"
#include "LedController.h"

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
    ClockFace(LedController& ledController, In14NixieTube& nixieTube);
    void Handle(uint32_t& tick);
    /**
     * @brief  Show time on the nixie tube
     *
     * @param now The current time
     * @param times The number of times the time is shown on the nixie
     * tube
     */
    void ShowTime(RtcDateTime now, uint8_t times = 1);
};