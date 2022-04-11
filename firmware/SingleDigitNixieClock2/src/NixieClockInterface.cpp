#include "NixieClockInterface.h"
#include "ConfigStore.h"

NixieClockInterface::NixieClockInterface(LedController& lc)
:ledController(lc)
{
}

LedInfo NixieClockInterface::OnGetBacklightData() const
{
    return ledController.GetLedInfo();
}
bool NixieClockInterface::OnSetBacklightState(uint8_t state)
{
    if(state >= static_cast<uint8_t>(LedState::MAX))
    {
        return false;
    }
    ledController.GetLedInfo().SetState(static_cast<LedState>(state));
    ConfigStore::SaveLedConfiguration(ledController.GetLedInfo());
    return true;
}