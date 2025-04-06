#include "TimeInfo.h"

TimeInfo::TimeInfo(int offset, bool isDst) : offset(offset), isDst(isDst) {}

int TimeInfo::GetOffset() const { return offset; }

void TimeInfo::SetOffset(int value) { offset = value; }

bool TimeInfo::IsDst() const { return isDst; }

void TimeInfo::SetDst(bool value) { isDst = value; }

JsonDocument TimeInfo::ToJson() const {
    JsonDocument doc;
    doc["offset"] = offset;
    doc["isDst"] = static_cast<uint8_t>(isDst);
    return doc;
}