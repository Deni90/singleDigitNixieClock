#include "ClockFace.h"
#include "ConfigStore.h"

namespace {
constexpr uint16_t ANIMATION_PERIOD = 50;
constexpr uint16_t DIGIT_DURATION = 300;
constexpr uint16_t PAUSE_DURATION = 2000;
}   // namespace

ClockFace::ClockFace(LedController& ledController, In14NixieTube& nixieTube)
    : animationState(AnimationStates::IDLE), ledController(ledController),
      nixieTube(nixieTube) {}

void ClockFace::Handle(uint32_t& tick) {
    static uint8_t animationframe = 0;
    static uint8_t pauseCounter = 0;

    switch (animationState) {
    default:
    case AnimationStates::IDLE:
        // do nothing
        break;
    case AnimationStates::INIT: {
        if (ledController.GetLedInfo().GetState() > LedState::ON) {
            LedInfo li = ledController.GetLedInfo();
            li.SetState(LedState::ON);
            ledController.SetLedInfo(li);
        }
        ledController.Lock();
        animationState = AnimationStates::INTRO;
        break;
    }
    case AnimationStates::INTRO: {
        if (tick >= ANIMATION_PERIOD) {
            tick = 0;
            if (animationframe > 9)   // end of animation?
            {
                animationframe = 0;
                nixieTube.HideDigit();
                animationState = AnimationStates::SHOW_TIME;
                break;
            }
            nixieTube.ShowDigit(9 - animationframe);
            animationframe++;
        }
        break;
    }
    case AnimationStates::SHOW_TIME: {
        if (tick >= DIGIT_DURATION) {
            tick = 0;
            if (animationframe == 0) {
                nixieTube.ShowDigit(time.Hour() / 10);
            } else if (animationframe == 2) {
                nixieTube.ShowDigit(time.Hour() % 10);
            } else if (animationframe == 4) {
                nixieTube.ShowDigit(time.Minute() / 10);
            } else if (animationframe == 6) {
                nixieTube.ShowDigit(time.Minute() % 10);
            } else if (animationframe == 1 || animationframe == 3 ||
                       animationframe == 5) {
                nixieTube.HideDigit();
            } else {
                animationframe = 0;
                nixieTube.HideDigit();
                animationState = AnimationStates::PAUSE;
                break;
            }
            animationframe++;
        }
        break;
    }
    case AnimationStates::PAUSE: {
        if (pauseCounter >= repeatNumber - 1) {
            animationState = AnimationStates::CLEANUP;
        }
        if (tick >= PAUSE_DURATION)   // FIXME
        {
            tick = 0;
            pauseCounter++;
            animationState = AnimationStates::SHOW_TIME;
        }
        break;
    }
    case AnimationStates::CLEANUP:
        ledController.Unlock();
        LedInfo li;
        ConfigStore::LoadLedConfiguration(li);
        ledController.SetLedInfo(li);
        animationframe = 0;
        pauseCounter = 0;
        animationState = AnimationStates::IDLE;
        break;
    }
}

void ClockFace::ShowTime(RtcDateTime now, uint8_t repeat) {
    animationState = AnimationStates::INIT;
    time = now;
    repeatNumber = repeat;
}