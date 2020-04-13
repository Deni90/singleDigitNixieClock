#include "LedController.h"

LedController::LedController(Adafruit_NeoPixel& led_)
: led(led_)
, r(0)
, g(0)
, b(0)
, a(0)
, state(OFF)
, counter(0)
, direction(true)
{
    led.begin();
    led.setBrightness(a);
    led.setPixelColor(0, led.Color(r, g, b));
    led.show();
}

void LedController::Initialize(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_, LedState state_)
{
    r = r_;
    g = g_;
    b = b_;
    a = a_;
    state = state_;
}

void LedController::Reset()
{
    counter = 0;
    direction = true;
}

uint8_t LedController::GetR() const
{
    return r;  
}

void LedController::SetR(const uint8_t value_)
{
    r = value_;
}

uint8_t LedController::GetG() const
{
    return g;
}

void LedController::SetG(const uint8_t value_)
{
    g = value_;
}

uint8_t LedController::GetB() const
{
    return b;
}

void LedController::SetB(const uint8_t value_)
{
    b = value_;
}

uint8_t LedController::GetA() const
{
    return a;
}

void LedController::SetA(const uint8_t value_)
{
    a = value_;
}

LedState LedController::GetState() const
{
    return state;
}

void LedController::SetState(const LedState value_)
{
    state = value_;
}

uint8_t LedController::CalculateNewColor(uint8_t color, uint8_t brightness)
{
    return (color * brightness) / 255;
}

void LedController::SetColor(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_)
{
    r = r_;
    g = g_;
    b = b_;
    a = a_;
}

void LedController::UpdateState()
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

    switch(state)
    {
        case OFF:
        {
            led.setBrightness(0);
            break;
        }
        case SOLID:
        {
            led.setBrightness(a);
            uint32_t color = led.Color(r, g, b);
            led.setPixelColor(0, color);
            break;
        }
        case BREATHE:
        {
            uint8_t new_a = CalculateNewColor(a, counter);  
            uint8_t new_r = CalculateNewColor(r, new_a);
            uint8_t new_g = CalculateNewColor(g, new_a);
            uint8_t new_b = CalculateNewColor(b, new_a);
            uint32_t color = led.Color(new_r, new_g, new_b);
            led.setBrightness(a);
            led.setPixelColor(0, led.gamma32(color));
            break;
        }
    }
    led.show();
}