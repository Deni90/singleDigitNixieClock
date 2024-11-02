#include "NixieClock.h"
#include "ConfigStore.h"

namespace {
constexpr uint16_t ANIMATION_PERIOD = 50;
constexpr uint16_t DIGIT_DURATION = 300;
constexpr uint16_t PAUSE_DURATION = 2000;
}   // namespace

NixieClock::NixieClock(LedController& ledController, In14NixieTube& nixieTube,
                       RtcDS3231<TwoWire>& rtc)
    : animationState(AnimationStates::IDLE), ledController(ledController),
      nixieTube(nixieTube), rtc(rtc) {
    ConfigStore::LoadSleepInfo(sleepInfo);
    Serial.printf("NixieClock() - SleepInfo: sb = %d\tsa = %d\n",
                  sleepInfo.GetSleepBefore(), sleepInfo.GetSleepAfter());
}

void NixieClock::Initialize(RtcDateTime time) {
    this->time = time;
    animationState = AnimationStates::INIT;
}

void NixieClock::Handle(uint32_t& tick) {
    static uint8_t animationframe = 0;
    static uint8_t pauseCounter = 0;

    switch (animationState) {
    default:
    case AnimationStates::IDLE:
        // do nothing
        break;
    case AnimationStates::INIT: {
        LedInfo li;
        ConfigStore::LoadLedInfo(li);
        if (IsInSleepMode()) {
            Serial.println("Sleep mode");
            li.SetState(LedState::OFF);
            ledController.SetLedInfo(li);
            animationState = AnimationStates::IDLE;
            break;
        }
        if (li.GetState() > LedState::ON) {
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
        ConfigStore::LoadLedInfo(li);
        ledController.SetLedInfo(li);
        animationframe = 0;
        pauseCounter = 0;
        animationState = AnimationStates::IDLE;
        break;
    }
}

void NixieClock::ShowTime(RtcDateTime now, uint8_t repeat) {
    animationState = AnimationStates::INIT;
    time = now;
    repeatNumber = repeat;
}

LedInfo NixieClock::OnGetLedInfo() const {
    LedInfo li;
    ConfigStore::LoadLedInfo(li);
    return li;
}

void NixieClock::OnSetLedInfo(const LedInfo& ledInfo) {
    ledController.SetLedInfo(ledInfo);
    ConfigStore::SaveLedInfo(ledInfo);
}

void NixieClock::OnSetCurrentTime(uint16_t year, uint8_t month,
                                  uint8_t dayOfMonth, uint8_t hour,
                                  uint8_t minute, uint8_t second) {
    Serial.printf("Set current date and time: %d/%d/%d %02d:%02d:%02d\n", year,
                  month, dayOfMonth, hour, minute, second);
    rtc.SetDateTime(RtcDateTime(year, month, dayOfMonth, hour, minute, second));
    ShowTime(rtc.GetDateTime());
}

SleepInfo NixieClock::OnGetSleepInfo() const {
    SleepInfo si;
    ConfigStore::LoadSleepInfo(si);
    return si;
}

void NixieClock::OnSetSleepInfo(const SleepInfo& sleepInfo) {
    this->sleepInfo = sleepInfo;
    ConfigStore::SaveSleepInfo(sleepInfo);
    Serial.printf("OnSetSleepInfo() - SleepInfo: sb = %d\tsa = %d\n",
                  this->sleepInfo.GetSleepBefore(),
                  this->sleepInfo.GetSleepAfter());
}

bool NixieClock::IsInSleepMode() {
    if (sleepInfo.GetSleepBefore() < sleepInfo.GetSleepAfter()) {
        if (time.Hour() < sleepInfo.GetSleepBefore() ||
            time.Hour() > sleepInfo.GetSleepAfter()) {
            return true;
        }
    } else {
        if (time.Hour() <= sleepInfo.GetSleepBefore() ||
            time.Hour() >= sleepInfo.GetSleepAfter()) {
            return true;
        }
    }
    return false;
}