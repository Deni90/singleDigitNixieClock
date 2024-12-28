#pragma once

#include <ArduinoJson.h>
#include <inttypes.h>

/**
 * @brief An enumeration representing states of the RGB led
 */
enum class LedState { OFF = 0, ON, FADE, PULSE, MAX };

/**
 * @brief Represents an RGB LED
 *
 */
class LedInfo {
  private:
    uint8_t r;
    uint8_t g;
    uint8_t b;
    LedState state;

  public:
    /**
     * @brief Default constructor
     */
    LedInfo();

    /**
     * @brief Construct a new Led Info object
     *
     * @param r red value
     * @param g green value
     * @param b blue value
     * @param state state of the LED
     */
    LedInfo(uint8_t r, uint8_t g, uint8_t b, LedState state);

    /**
     * @brief Default destructor
     */
    ~LedInfo() = default;

    /**
     * @brief Default copy constructor
     * @param ledInfo LedInfo object
     */
    LedInfo(const LedInfo& ledInfo) = default;

    /**
     * @brief Getter for red value
     *
     * @return uint8_t value
     */
    uint8_t GetR() const;

    /**
     * @brief Setter for red value
     *
     * @param value value
     */
    void SetR(const uint8_t value);

    /**
     * @brief Getter for green value
     *
     * @return uint8_t value
     */
    uint8_t GetG() const;

    /**
     * @brief Setter for green value
     *
     * @param value value
     */
    void SetG(const uint8_t value);

    /**
     * @brief Getter for blue value
     *
     * @return uint8_t value
     */
    uint8_t GetB() const;

    /**
     * @brief Setter for blue value
     *
     * @param value value
     */
    void SetB(const uint8_t value);

    /**
     * @brief Getter for the State object
     *
     * @return LedState state
     */
    LedState GetState() const;

    /**
     * @brief Setter for the State object
     *
     * @param state LedState
     */
    void SetState(const LedState state);

    /**
     * @brief Set color
     *
     * @param r red value
     * @param g green value
     * @param b blue value
     */
    void SetColor(uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Convert LedInfo to JSON format
     *
     * @return JsonDocument JSON object containing LedInfo
     */
    JsonDocument ToJson() const;
};