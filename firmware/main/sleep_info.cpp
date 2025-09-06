/******************************************************************************
 * File:    sleep_info.cpp
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Implements SleepInfo data class
 ******************************************************************************/

#include "sleep_info.h"

SleepInfo::SleepInfo(uint16_t mSleepBefore, uint16_t mSleepAfter)
    : mSleepBefore(mSleepBefore), mSleepAfter(mSleepAfter) {}

uint16_t SleepInfo::getSleepBefore() const { return mSleepBefore; }

void SleepInfo::setSleepBefore(const uint16_t value) { mSleepBefore = value; }

uint16_t SleepInfo::getSleepAfter() const { return mSleepAfter; }

void SleepInfo::setSleepAfter(const uint16_t value) { mSleepAfter = value; }