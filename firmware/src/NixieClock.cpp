#include "NixieClock.h"
#include "ConfigStore.h"

namespace {
constexpr uint16_t ANIMATION_PERIOD = 50;
constexpr uint16_t DIGIT_DURATION = 300;
constexpr uint16_t PAUSE_DURATION = 2000;
}   // namespace

NixieClock::NixieClock(LedController& ledController, In14NixieTube& nixieTube,
                       RtcDS3231<TwoWire>& rtc, TimeManager& timeManager)
    : animationState(AnimationStates::IDLE), ledController(ledController),
      nixieTube(nixieTube), rtc(rtc), timeManager(timeManager) {}

void NixieClock::Initialize(const SleepInfo& sleepInfo) {
    this->sleepInfo = sleepInfo;
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
        // Start time animation
        // If configured turn on LED
        LedInfo li;
        ConfigStore::LoadLedInfo(li);
        if (IsInSleepMode()) {
            Serial.printf("%02d/%02d/%04d %02d:%02d:%02d - Sleep mode\n",
                          time.Day(),      // get day method
                          time.Month(),    // get month method
                          time.Year(),     // get year method
                          time.Hour(),     // get hour method
                          time.Minute(),   // get minute method
                          time.Second()    // get second method
            );
            li.SetState(LedState::OFF);
            ledController.SetLedInfo(li);
            animationState = AnimationStates::IDLE;
            break;
        }
        Serial.printf(
            "%02d/%02d/%04d %02d:%02d:%02d - Show time on nixie tube\n",
            time.Day(),      // get day method
            time.Month(),    // get month method
            time.Year(),     // get year method
            time.Hour(),     // get hour method
            time.Minute(),   // get minute method
            time.Second()    // get second method
        );
        if (li.GetState() > LedState::ON) {
            li.SetState(LedState::ON);
            ledController.SetLedInfo(li);
        }
        ledController.Lock();
        animationState = AnimationStates::INTRO;
        break;
    }
    case AnimationStates::INTRO: {
        // Show a fast countdown of digits from 9 to 0 on nixie tube as an intro
        // before showing the actual time
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
        // Show time on nixie tube. Since there is only one nixie tube it is
        // needed to sequentially show digits. The time is displayed in the
        // following format: H <pause> H <pause> M <pause> M
        if (tick >= DIGIT_DURATION) {
            tick = 0;
            // animationframe
            // Value   | Description
            // 0       | Hour
            // 2       | Hour
            // 4       | Minute
            // 6       | Minute
            // 1, 3, 5 | Blank, pause between showing two digits
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
        // This state represents a pause between repeates
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
        // End of the animation
        // Restore led state to the state before animation
        // Reset vatiables
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
    if (!IsInSleepMode()) {
        ledController.SetLedInfo(ledInfo);
    }
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
    LedInfo li;
    ConfigStore::LoadLedInfo(li);
    if (IsInSleepMode()) {
        // turn off the led
        li.SetState(LedState::OFF);
        ledController.SetLedInfo(li);
    } else {
        // restore the previous state
        if (animationState == AnimationStates::IDLE) {
            ledController.SetLedInfo(li);
        }
    }
}

WifiInfo NixieClock::OnGetWifiInfo() const {
    WifiInfo wi;
    ConfigStore::LoadWifiInfo(wi);
    return wi;
}

void NixieClock::OnSetWifiInfo(const WifiInfo& wifiInfo) {
    ConfigStore::SaveWifiInfo(wifiInfo);
    ESP.restart();
}

TimeInfo NixieClock::OnGetTimeInfo() const {
    TimeInfo ti;
    ConfigStore::LoadTimeInfo(ti);
    return ti;
}

void NixieClock::OnSetTimeInfo(const TimeInfo& timeInfo) {
    ConfigStore::SaveTimeInfo(timeInfo);
    int offset = timeInfo.GetOffset() + (timeInfo.IsDst() ? 3600 : 0);
    timeManager.SetOffset(offset);
}

bool NixieClock::IsInSleepMode() {
    uint16_t timeInMinutes = time.Hour() * 60 + time.Minute();
    if (sleepInfo.GetSleepBefore() < sleepInfo.GetSleepAfter()) {
        if (timeInMinutes < sleepInfo.GetSleepBefore() ||
            timeInMinutes > sleepInfo.GetSleepAfter()) {
            return true;
        }
    } else {
        if (timeInMinutes < sleepInfo.GetSleepBefore() &&
            timeInMinutes > sleepInfo.GetSleepAfter()) {
            return true;
        }
    }
    return false;
}