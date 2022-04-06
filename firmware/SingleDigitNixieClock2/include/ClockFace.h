#pragma once

#include <inttypes.h>
#include <RtcDS3231.h>
#include "LedController.h"
#include "BCD2DecimalDecoder.h"

class ClockFace
{
private:
    enum class AnimationStates{
        IDLE = 0,
        INIT,
        INTRO,
        SHOW_TIME,
        PAUSE,
        CLEANUP
    };
    AnimationStates animationState;
    LedController& ledController;
    BCD2DecimalDecoder nixieTube;
    RtcDateTime time;
public:
    ClockFace(LedController& ledController, BCD2DecimalDecoder& nixieTube);
    void Handle(uint32_t& tick);
    void ShowTime(RtcDateTime now);
};