#include "NixieClockInterface.h"
#include "ConfigStore.h"

NixieClockInterface::NixieClockInterface(LedController& ledController,
                                         RtcDS3231<TwoWire>& rtc,
                                         ClockFace& clock)
    : ledController(ledController), rtc(rtc), clock(clock) {}

LedInfo NixieClockInterface::OnGetLedInfo() const {
    return ledController.GetLedInfo();
}

void NixieClockInterface::OnSetLedInfo(const LedInfo& ledInfo) {
    ledController.SetLedInfo(ledInfo);
    ConfigStore::SaveLedConfiguration(ledInfo);
}

void NixieClockInterface::OnSetCurrentTime(uint16_t year, uint8_t month,
                                           uint8_t dayOfMonth, uint8_t hour,
                                           uint8_t minute, uint8_t second) {
    Serial.printf("Set current date and time: %d/%d/%d %d:%d:%d\n", year, month,
                  dayOfMonth, hour, minute, second);
    rtc.SetDateTime(RtcDateTime(year, month, dayOfMonth, hour, minute, second));
    clock.ShowTime(rtc.GetDateTime());
}