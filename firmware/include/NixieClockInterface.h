#pragma once

#include "ClockInterface.h"
#include "LedController.h"

class NixieClockInterface : public ClockInterface
{
private:
    LedController &ledController;

public:
    NixieClockInterface(LedController &ledController);

    LedInfo OnGetBacklightData() const;
    bool OnSetBacklightState(uint8_t state);
    void OnSetBacklightColor(uint8_t r, uint8_t g, uint8_t b);
};