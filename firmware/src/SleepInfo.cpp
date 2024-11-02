#include "SleepInfo.h"

SleepInfo::SleepInfo(uint8_t sleepBefore, uint8_t sleepAfter)
    : sleepBefore(sleepBefore), sleepAfter(sleepAfter) {}

uint8_t SleepInfo::GetSleepBefore() const { return sleepBefore; }

void SleepInfo::SetSleepBefore(const uint8_t value) { sleepBefore = value; }

uint8_t SleepInfo::GetSleepAfter() const { return sleepAfter; }

void SleepInfo::SetSleepAfter(const uint8_t value) { sleepAfter = value; }