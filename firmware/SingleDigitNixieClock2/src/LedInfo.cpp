#include "LedInfo.h"

LedInfo::LedInfo()
    : r(0), g(0), b(0), state(LedState::OFF)
{
}

LedInfo::LedInfo(uint8_t r, uint8_t g, uint8_t b, LedState state)
{
    this->r = r;
    this->g = g;
    this->b = b;
    this->state = state;
}

uint8_t LedInfo::GetR() const
{
    return r;
}

void LedInfo::SetR(const uint8_t value)
{
    r = value;
}

uint8_t LedInfo::GetG() const
{
    return g;
}

void LedInfo::SetG(const uint8_t value)
{
    g = value;
}

uint8_t LedInfo::GetB() const
{
    return b;
}

void LedInfo::SetB(const uint8_t value)
{
    b = value;
}

LedState LedInfo::GetState() const
{
    return state;
}

void LedInfo::SetState(const LedState state)
{
    this->state = state;
}

void LedInfo::SetColor(uint8_t r, uint8_t g, uint8_t b)
{
    this->r = r;
    this->g = g;
    this->b = b;
}