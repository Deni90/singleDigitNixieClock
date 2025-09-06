/******************************************************************************
 * File:    led_info.h
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Declaration of a data class used for LED config
 ******************************************************************************/

#ifndef led_info_h
#define led_info_h

#include <inttypes.h>

/**
 * @brief An enumeration representing states of the RGB led
 */
enum class LedState { Off, On, Fade, Pulse };

/**
 * @brief Convert led state to c-style string
 * @param[in] led state
 */
constexpr const char* ledStateToString(LedState ls) {
    switch (ls) {
    case LedState::Off:
        return "off";
    case LedState::On:
        return "on";
    case LedState::Fade:
        return "fade";
    case LedState::Pulse:
        return "pulse";
    default:
        return "unknown";
    }
}

/**
 * @brief Represents an RGB LED
 *
 */
class LedInfo {
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
    uint8_t getRed() const;

    /**
     * @brief Setter for red value
     *
     * @param value value
     */
    void setRed(const uint8_t value);

    /**
     * @brief Getter for green value
     *
     * @return uint8_t value
     */
    uint8_t getGreen() const;

    /**
     * @brief Setter for green value
     *
     * @param value value
     */
    void setGreen(const uint8_t value);

    /**
     * @brief Getter for blue value
     *
     * @return uint8_t value
     */
    uint8_t getBlue() const;

    /**
     * @brief Setter for blue value
     *
     * @param value value
     */
    void setBlue(const uint8_t value);

    /**
     * @brief Getter for the State object
     *
     * @return LedState state
     */
    LedState getState() const;

    /**
     * @brief Setter for the State object
     *
     * @param state LedState
     */
    void setState(const LedState state);

    /**
     * @brief Set color
     *
     * @param r red value
     * @param g green value
     * @param b blue value
     */
    void setColor(uint8_t r, uint8_t g, uint8_t b);

  private:
    uint8_t mRed;
    uint8_t mGreen;
    uint8_t mBlue;
    LedState mState;
};

#endif   // led_info_h