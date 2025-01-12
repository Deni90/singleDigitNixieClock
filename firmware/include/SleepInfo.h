#pragma once

#include <ArduinoJson.h>
#include <inttypes.h>

/**
 * @brief Represents a Sleep info class
 *
 */
class SleepInfo {
  private:
    uint16_t sleepBefore;
    uint16_t sleepAfter;

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
    SleepInfo(uint16_t sleepBefore, uint16_t sleepAfter);

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
     * @return uint16_t value
     */
    uint16_t GetSleepBefore() const;

    /**
     * @brief Setter for sleep before
     *
     * @param value value
     */
    void SetSleepBefore(const uint16_t value);

    /**
     * @brief Getter for sleep before
     *
     * @return uint16_t value
     */
    uint16_t GetSleepAfter() const;

    /**
     * @brief Setter for sleep before
     *
     * @param value value
     */
    void SetSleepAfter(const uint16_t value);

    /**
     * @brief Convert SleepInfo to JSON format
     *
     * @return JsonDocument JSON object containing SleepInfo
     */
    JsonDocument ToJson() const;
};