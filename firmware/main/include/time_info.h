/******************************************************************************
 * File:    time_info.h
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Declaration of a data class used for clock related config
 ******************************************************************************/

#ifndef time_info_h
#define time_info_h

#include <inttypes.h>
#include <string>

/**
 * @brief An enumeration representing time formats
 */
enum class TimeFormat { Hour24, Hour12 };

/**
 * @brief Convert time format to c-style string
 * @param[in] tf time format
 */
constexpr const char* timeFormatToString(TimeFormat tf) {
    switch (tf) {
    case TimeFormat::Hour24:
        return "24h";
    case TimeFormat::Hour12:
        return "12h";
    default:
        return "unknown";
    }
}

/**
 * @brief Represents a data class used for storing time related config
 *
 */
class TimeInfo {
  public:
    /**
     * @brief Default constructor
     */
    TimeInfo() = default;

    /**
     * @brief Construct a new Time Info object
     *
     * @param[in] tzZone Time zone string in geographical format
     * @param[in] tzOffset Time zone offset in proleptic format
     */
    TimeInfo(const std::string& tzZone, const std::string& tzOffset);

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
     * @brief Getter for zone
     *
     * @return zone
     */
    std::string getTzZone() const;

    /**
     * @brief Setter for zone
     *
     * @param value zone string
     */
    void setTzZone(const std::string& value);

    /**
     * @brief Getter for offset
     *
     * @return offset
     */
    std::string getTzOffset() const;

    /**
     * @brief Setter for offset
     *
     * @param value offset string
     */
    void setTzOffset(const std::string& value);

    /**
     * @brief Getter for time format
     *
     * @return time format
     */
    TimeFormat getTimeFormat() const;

    /**
     * @brief Setter for time format
     *
     * @param value time format
     */
    void setTimeFormat(TimeFormat value);

  private:
    std::string mTzZone;
    std::string mTzOffset;
    TimeFormat mTimeFormat;
};

#endif   // time_info_h