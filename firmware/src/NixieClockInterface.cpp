#include "NixieClockInterface.h"
#include "ConfigStore.h"

NixieClockInterface::NixieClockInterface(LedController &ledController)
    : ledController(ledController)
{
}

LedInfo NixieClockInterface::OnGetBacklightData() const
{
    return ledController.GetLedInfo();
}
bool NixieClockInterface::OnSetBacklightState(uint8_t state)
{
    if (state >= static_cast<uint8_t>(LedState::MAX))
    {
        return false;
    }
    ledController.GetLedInfo().SetState(static_cast<LedState>(state));
    ConfigStore::SaveLedConfiguration(ledController.GetLedInfo());
    return true;
}

void NixieClockInterface::OnSetBacklightColor(uint8_t r, uint8_t g, uint8_t b)
{
    if (ledController.GetLedInfo().GetR() == r && ledController.GetLedInfo().GetG() == g && ledController.GetLedInfo().GetB() == b)
    {
        // There is no need to save same data
        return;
    }
    ledController.GetLedInfo().SetColor(r, g, b);
    ConfigStore::SaveLedConfiguration(ledController.GetLedInfo());
}