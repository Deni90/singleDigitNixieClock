#pragma once

#include <inttypes.h>
#include <Adafruit_NeoPixel.h>
#include "LedState.h"

class LedController
{
private:
    Adafruit_NeoPixel& led;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
    LedState state;
    uint8_t counter;
    bool direction;
    uint8_t CalculateNewColor(uint8_t color, uint8_t brightness);
public:
    LedController(Adafruit_NeoPixel& led_);
    void Initialize(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_, LedState state_);
    void Reset();
    uint8_t GetR() const;
    void SetR(const uint8_t value_);
    uint8_t GetG() const;
    void SetG(const uint8_t value_);
    uint8_t GetB() const;
    void SetB(const uint8_t value_);
    uint8_t GetA() const;
    void SetA(const uint8_t value_);
    LedState GetState() const;
    void SetState(const LedState value_);
    void SetColor(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_);
    void UpdateState();
};