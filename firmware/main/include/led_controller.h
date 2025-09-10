/******************************************************************************
 * File:    led_controller.h
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Declaration of a class used for controlling RGB LED
 ******************************************************************************/

#ifndef led_controller_h
#define led_controller_h

#include <inttypes.h>
#include <optional>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "led_info.h"
#include "led_strip.h"

/**
 * @brief A driver class for handling the RGB led
 *
 */
class LedController {
  public:
    /**
     * @brief Construct a new Led Controller object
     *
     * @param ledPin data pin of the RGB LED
     */
    LedController(gpio_num_t ledPin);

    /**
     * @brief Initialize the LED with led info
     *
     * @param ledInfo led info
     */
    void initialize(LedInfo ledInfo);

    /**
     * @brief This is the loop method of the class.
     *
     * It is updating the actual led state (color and brightness)accoring to led
     * state.
     *
     */
    void update();

    /**
     * @brief Set the Led Info object
     *
     * @param li led info
     */
    void setLedInfo(const LedInfo& li);

    /**
     * @brief Get the Led Info object
     *
     * @return led info
     */
    LedInfo getLedInfo();

    /**
     * @brief Set temporal state of the LED
     *
     * @param[in] LED state
     */
    void setTemporalState(const LedInfo& state);

    /**
     * @brief Clear temporal led state.
     */
    void clearTemporalState();

  private:
    void test();

    gpio_num_t mLedPin;
    led_strip_handle_t mLedHandle;
    LedInfo mLedInfo;
    uint8_t mCounter;
    bool mDirection;
    std::optional<LedInfo> mTempState;
    SemaphoreHandle_t mMutex;
};

#endif   // led_controller_h