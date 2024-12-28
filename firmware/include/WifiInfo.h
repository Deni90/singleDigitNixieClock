#pragma once

#include <ArduinoJson.h>
#include <inttypes.h>

/**
 * @brief Represents a Wifi info class
 *
 */
class WifiInfo {
  private:
    String ssid;
    String password;

  public:
    /**
     * @brief Default constructor
     */
    WifiInfo() = default;

    /**
     * @brief Construct a new Sleep Info object
     *
     * @param ssid AP SSID
     * @param password password
     */
    WifiInfo(const String& ssid, const String& password);

    /**
     * @brief Default destructor
     */
    ~WifiInfo() = default;

    /**
     * @brief Default copy constructor
     * @param ledInfo LedInfo object
     */
    WifiInfo(const WifiInfo& WifiInfo) = default;

    /**
     * @brief Getter for ssid
     *
     * @return String SSID
     */
    String GetSSID() const;

    /**
     * @brief Setter for SSID
     *
     * @param value ssid
     */
    void SetSSID(const String& value);

    /**
     * @brief Getter for password
     *
     * @return String base64 encoded password
     */
    String GetPassword() const;

    /**
     * @brief Setter for password
     *
     * @param value base64 encoded password
     */
    void SetPassword(const String& value);

    /**
     * @brief Convert WifiInfo to JSON format
     *
     * @return JsonDocument JSON object containing WifiInfo
     */
    JsonDocument ToJson() const;
};