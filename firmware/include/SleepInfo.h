#pragma once

#include <ArduinoJson.h>
#include <inttypes.h>

/**
 * @brief Represents a Sleep info class
 *
 */
class SleepInfo {
  private:
    uint8_t sleepBefore;
    uint8_t sleepAfter;

  public:
    /**
     * @brief Default constructor
     */
    SleepInfo() = default;

    /**
     * @brief Construct a new Sleep Info object
     *
     * @param sleepBefore sleep before hour
     * @param sleepAfter sleep after hour
     */
    SleepInfo(uint8_t sleepBefore, uint8_t sleepAfter);

    /**
     * @brief Default destructor
     */
    ~SleepInfo() = default;

    /**
     * @brief Default copy constructor
     * @param ledInfo LedInfo object
     */
    SleepInfo(const SleepInfo& sleepInfo) = default;

    /**
     * @brief Getter for sleep before
     *
     * @return uint8_t value
     */
    uint8_t GetSleepBefore() const;

    /**
     * @brief Setter for sleep before
     *
     * @param value value
     */
    void SetSleepBefore(const uint8_t value);

    /**
     * @brief Getter for sleep before
     *
     * @return uint8_t value
     */
    uint8_t GetSleepAfter() const;

    /**
     * @brief Setter for sleep before
     *
     * @param value value
     */
    void SetSleepAfter(const uint8_t value);

    /**
     * @brief Convert SleepInfo to JSON format
     *
     * @return JsonDocument JSON object containing SleepInfo
     */
    JsonDocument ToJson() const;
};