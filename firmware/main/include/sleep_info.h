/******************************************************************************
 * File:    sleep_info.h
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Declaration of a data class used for sleep mode config
 ******************************************************************************/

#ifndef sleep_info_h
#define sleep_info_h

#include <inttypes.h>

/**
 * @brief Represents a Sleep info class
 *
 */
class SleepInfo {
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
    uint16_t getSleepBefore() const;

    /**
     * @brief Setter for sleep before
     *
     * @param value value
     */
    void setSleepBefore(const uint16_t value);

    /**
     * @brief Getter for sleep before
     *
     * @return uint16_t value
     */
    uint16_t getSleepAfter() const;

    /**
     * @brief Setter for sleep before
     *
     * @param value value
     */
    void setSleepAfter(const uint16_t value);

  private:
    uint16_t mSleepBefore;
    uint16_t mSleepAfter;
};

#endif   // sleep_info_h