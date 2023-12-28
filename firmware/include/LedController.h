#pragma once

#include "LedInfo.h"
#include <Adafruit_NeoPixel.h>
#include <inttypes.h>

class LedController {
  private:
    Adafruit_NeoPixel ledDriver;
    LedInfo ledInfo;
    uint8_t counter;
    bool direction;

  public:
    LedController(uint16_t ledPin);
    void Initialize(LedInfo ledInfo);
    void Reset();
    void Update();
    void SetLedInfo(const LedInfo li);
    LedInfo& GetLedInfo();
};