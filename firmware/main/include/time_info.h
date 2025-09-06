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
 * @brief Represents a Wifi info class
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

  private:
    std::string mTzZone;
    std::string mTzOffset;
};

#endif   // time_info_h