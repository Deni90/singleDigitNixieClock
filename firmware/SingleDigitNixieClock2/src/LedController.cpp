#include "LedController.h"

namespace{
    constexpr uint16_t LED_COUNT = 1;
}

LedController::LedController(uint16_t ledPin)
: ledDriver(LED_COUNT, ledPin, NEO_GRB + NEO_KHZ800)
, counter(0)
, direction(true)
{
}

void LedController::Initialize(LedInfo ledInfo)
{
    this->ledInfo = ledInfo;
    ledDriver.begin();
    ledDriver.setBrightness(ledInfo.GetA());
    ledDriver.setPixelColor(0, ledDriver.Color(ledInfo.GetR(), ledInfo.GetG(), ledInfo.GetB()));
    ledDriver.show();
}

void LedController::Reset()
{
    counter = 0;
    direction = true;
}

uint8_t LedController::CalculateNewColor(uint8_t color, uint8_t brightness)
{
    return (color * brightness) / 255;
}

void LedController::Update()
{
    if(direction)
    {
        if(counter < 255)
        {
            counter++;
        }
        else
        {
            direction = false;
        }
    }
    else
    {
        if(counter == 0)
        {
        direction = true;
        }
        else
        {
        counter--;
        }
    }

    switch(ledInfo.GetState())
    {
        case LedState::OFF:
        {
            ledDriver.setBrightness(0);
            break;
        }
        case LedState::ON:
        {
            ledDriver.setBrightness(ledInfo.GetA());
            uint32_t color = ledDriver.Color(ledInfo.GetR(), ledInfo.GetG(), ledInfo.GetB());
            ledDriver.setPixelColor(0, color);
            break;
        }
        case LedState::FADE:
        {
            uint8_t new_a = CalculateNewColor(ledInfo.GetA(), counter);
            uint8_t new_r = CalculateNewColor(ledInfo.GetR(), new_a);
            uint8_t new_g = CalculateNewColor(ledInfo.GetG(), new_a);
            uint8_t new_b = CalculateNewColor(ledInfo.GetB(), new_a);
            uint32_t color = ledDriver.Color(new_r, new_g, new_b);
            ledDriver.setBrightness(ledInfo.GetA());
            ledDriver.setPixelColor(0, ledDriver.gamma32(color));
            break;
        }
    }
    ledDriver.show();
}

void LedController::SetLedInfo(const LedInfo li)
{
    ledInfo = li;
}
LedInfo& LedController::GetLedInfo()
{
    return ledInfo;
}