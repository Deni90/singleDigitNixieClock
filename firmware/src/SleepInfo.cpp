#include "SleepInfo.h"

SleepInfo::SleepInfo(uint16_t sleepBefore, uint16_t sleepAfter)
    : sleepBefore(sleepBefore), sleepAfter(sleepAfter) {}

uint16_t SleepInfo::GetSleepBefore() const { return sleepBefore; }

void SleepInfo::SetSleepBefore(const uint16_t value) { sleepBefore = value; }

uint16_t SleepInfo::GetSleepAfter() const { return sleepAfter; }

void SleepInfo::SetSleepAfter(const uint16_t value) { sleepAfter = value; }

JsonDocument SleepInfo::ToJson() const {
    JsonDocument doc;
    doc["sleep_before"] = sleepBefore;
    doc["sleep_after"] = sleepAfter;
    return doc;
}