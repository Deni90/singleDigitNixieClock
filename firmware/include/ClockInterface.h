#pragma once

#include <inttypes.h>

#include "LedInfo.h"

class ClockInterface {
  public:
    virtual ~ClockInterface() = default;
    virtual LedInfo OnGetBacklightData() const = 0;
    virtual bool OnSetBacklightState(uint8_t state) = 0;
    virtual void OnSetBacklightColor(uint8_t r, uint8_t g, uint8_t b) = 0;
    virtual void OnSetCurrentTime(uint16_t year, uint8_t month,
                                  uint8_t dayOfMonth, uint8_t hour,
                                  uint8_t minute, uint8_t second) = 0;
};