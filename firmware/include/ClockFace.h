#pragma once

#include <RtcDS3231.h>
#include <inttypes.h>

#include "In18NixieTube.h"
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
    In18NixieTube& nixieTube;
    RtcDateTime time;

  public:
    ClockFace(LedController& ledController, In18NixieTube& nixieTube);
    void Handle(uint32_t& tick);
    void ShowTime(RtcDateTime now);
};