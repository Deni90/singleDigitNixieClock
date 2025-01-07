#pragma once

#include <ArduinoJson.h>
#include <inttypes.h>

/**
 * @brief Represents a Wifi info class
 *
 */
class TimeInfo {
  private:
    int offset;

  public:
    /**
     * @brief Default constructor
     */
    TimeInfo() = default;

    /**
     * @brief Construct a new Time Info object
     *
     * @param offset offset in seconds
     */
    TimeInfo(int offset);

    /**
     * @brief Default destructor
     */
    ~TimeInfo() = default;

    /**
     * @brief Default copy constructor
     * @param timeInfo TimeInfo object
     */
    TimeInfo(const TimeInfo& TimeInfo) = default;

    /**
     * @brief Getter for offset
     *
     * @return offset
     */
    int GetOffset() const;

    /**
     * @brief Setter for offset
     *
     * @param value offset in seconds
     */
    void SetOffset(int value);

    /**
     * @brief Convert TimeInfo to JSON format
     *
     * @return JsonDocument JSON object containing TimeInfo
     */
    JsonDocument ToJson() const;
};