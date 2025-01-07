#include "TimeManager.h"

namespace {
constexpr unsigned long JANUARY_FIRST_2000 = 946684800;
}   // namespace

TimeManager::TimeManager() : ntpClient(udp) {}

void TimeManager::Initialize(int offset) {
    ntpClient.begin();
    ntpClient.setTimeOffset(offset);
}

void TimeManager::SetOffset(int offset) { ntpClient.setTimeOffset(offset); }

bool TimeManager::GetDateTime(RtcDateTime& dateTime) {
    if (!ntpClient.forceUpdate()) {
        return false;
    }
    dateTime = RtcDateTime(ntpClient.getEpochTime() - JANUARY_FIRST_2000);
    return true;
}