#pragma once

#include "LedInfo.h"
#include <Adafruit_NeoPixel.h>
#include <inttypes.h>

/**
 * @brief A driver class for handling the RGB led
 *
 */
class LedController {
  private:
    Adafruit_NeoPixel ledDriver;
    LedInfo ledInfo;
    uint8_t counter;
    bool direction;

  public:
    /**
     * @brief Construct a new Led Controller object
     *
     * @param ledPin data pin of the RGB LED
     */
    LedController(uint16_t ledPin);

    /**
     * @brief Initialize the LED with led info
     *
     * @param ledInfo led info
     */
    void Initialize(LedInfo ledInfo);

    /**
     * @brief This is the loop method of the class.
     *
     * It is updating the actual led state (color and brightness)accoring to led
     * state.
     *
     */
    void Update();

    /**
     * @brief Set the Led Info object
     *
     * @param li led info
     */
    void SetLedInfo(const LedInfo li);

    /**
     * @brief Get the Led Info object
     *
     * @return LedInfo& reference to led info
     */
    LedInfo& GetLedInfo();
};