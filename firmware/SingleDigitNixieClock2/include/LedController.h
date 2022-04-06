#pragma once

#include <inttypes.h>
#include <Adafruit_NeoPixel.h>
#include "LedInfo.h"

class LedController
{
private:
    Adafruit_NeoPixel ledDriver;
    LedInfo ledInfo;
    uint8_t counter;
    bool direction;
    uint8_t CalculateNewColor(uint8_t color, uint8_t brightness);
public:
    LedController(uint16_t ledPin);
    void Initialize(LedInfo ledInfo);
    void Reset();
    void Update();
    void SetLedInfo(const LedInfo li);
    LedInfo& GetLedInfo();
};