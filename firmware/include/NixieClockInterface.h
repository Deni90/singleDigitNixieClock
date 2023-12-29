#pragma once

#include "ClockFace.h"
#include "ClockInterface.h"
#include "LedController.h"
#include <RtcDS3231.h>
#include <Wire.h>

class NixieClockInterface : public ClockInterface {
  private:
    LedController& ledController;
    RtcDS3231<TwoWire>& rtc;
    ClockFace& clock;

  public:
    NixieClockInterface(LedController& ledController, RtcDS3231<TwoWire>& rtc,
                        ClockFace& clock);

    LedInfo OnGetBacklightData() const;
    bool OnSetBacklightState(uint8_t state);
    void OnSetBacklightColor(uint8_t r, uint8_t g, uint8_t b);

    void OnSetCurrentTime(uint16_t year, uint8_t month, uint8_t dayOfMonth,
                          uint8_t hour, uint8_t minute, uint8_t second);
};