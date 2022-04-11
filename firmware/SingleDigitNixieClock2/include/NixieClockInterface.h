#pragma once

#include "ClockInterface.h"
#include "LedController.h"

class NixieClockInterface : public ClockInterface
{
private:
    LedController& ledController;
public:
    NixieClockInterface(LedController& lc);

    LedInfo OnGetBacklightData() const;
    bool OnSetBacklightState(uint8_t state);
};