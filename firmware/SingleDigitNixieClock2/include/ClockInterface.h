#pragma once

#include <inttypes.h>

#include "LedInfo.h"

class ClockInterface
{
public:
    virtual LedInfo OnGetBacklightData() const = 0;
    virtual bool OnSetBacklightState(uint8_t state) = 0;
};