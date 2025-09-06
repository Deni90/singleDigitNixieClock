/******************************************************************************
 * File:    time_info.cpp
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Implements TimeInfo data class
 ******************************************************************************/

#include "time_info.h"

TimeInfo::TimeInfo(const std::string& tzZone, const std::string& tzOffset)
    : mTzZone(tzZone), mTzOffset(tzOffset) {}

std::string TimeInfo::getTzZone() const { return mTzZone; }

void TimeInfo::setTzZone(const std::string& value) { mTzZone = value; }

std::string TimeInfo::getTzOffset() const { return mTzOffset; }

void TimeInfo::setTzOffset(const std::string& value) { mTzOffset = value; }