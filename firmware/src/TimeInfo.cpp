#include "TimeInfo.h"

TimeInfo::TimeInfo(int offset) : offset(offset) {}

int TimeInfo::GetOffset() const { return offset; }

void TimeInfo::SetOffset(int value) { offset = value; }

JsonDocument TimeInfo::ToJson() const {
    JsonDocument doc;
    doc["offset"] = offset;
    return doc;
}