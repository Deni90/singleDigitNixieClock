/******************************************************************************
 * File:    wifi_info.h
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Declaration of a data class used for wifi config
 ******************************************************************************/

#ifndef wifi_info_h
#define wifi_info_h

#include <inttypes.h>
#include <string>

/**
 * @brief Represents a Wifi info class
 *
 */
class WifiInfo {
  public:
    /**
     * @brief Default constructor
     */
    WifiInfo() = default;

    /**
     * @brief Construct a new Wifi Info object
     *
     * @param hostname hostname
     * @param ssid SSID
     * @param password password
     */
    WifiInfo(const std::string& hostname, const std::string& ssid,
             const std::string& password);

    /**
     * @brief Default destructor
     */
    ~WifiInfo() = default;

    /**
     * @brief Default copy constructor
     * @param other WifiInfo object
     */
    WifiInfo(const WifiInfo& other) = default;

    /**
     * @brief Default copy assignment constructor
     * @param other WifiInfo object
     */
    WifiInfo& operator=(const WifiInfo& other) = default;

    /**
     * @brief Getter for hostname
     *
     * @return hostname
     */
    std::string getHostname() const;

    /**
     * @brief Setter for hostname
     *
     * @param value hostname
     */
    void setHostname(const std::string& value);

    /**
     * @brief Getter for ssid
     *
     * @return SSID
     */
    std::string getSSID() const;

    /**
     * @brief Setter for SSID
     *
     * @param value ssid
     */
    void setSSID(const std::string& value);

    /**
     * @brief Getter for password
     *
     * @return base64 encoded password
     */
    std::string getPassword() const;

    /**
     * @brief Setter for password
     *
     * @param value base64 encoded password
     */
    void setPassword(const std::string& value);

  private:
    std::string mHostname;
    std::string mSsid;
    std::string mPassword;
};

#endif   // wifi_info_h