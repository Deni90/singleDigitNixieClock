#pragma once

#include <NTPClient.h>
#include <RtcDS3231.h>
#include <WiFiUdp.h>

class TimeManager {
  private:
    WiFiUDP udp;
    NTPClient ntpClient;
  public:
    TimeManager();
    void Initialize(int offset);
    void SetOffset(int offset);
    /**
     * @brief Get Time from NTP server
     *
     * @param[in] offset Offset in seconds depending on the timezone
     * @param[out] dateTime Populated RtcDateTime with result
     * @return true if the retrieval is successfull
     */
    bool GetDateTime(RtcDateTime& dateTime);
};