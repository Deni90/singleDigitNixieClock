/******************************************************************************
 * File:    led_controller.cpp
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Implements LedController class for controlling RGB LED
 ******************************************************************************/

#include "led_controller.h"

#include "driver/rmt_tx.h"
#include "led_strip.h"

static constexpr uint16_t kLedCount = 1;
static constexpr uint8_t kPulseTime = 10;
static constexpr uint8_t kMaxBrightness = 255;

LedController::LedController(gpio_num_t ledPin)
    : mLedPin(ledPin), mCounter(0), mDirection(true), mLock(false) {
    mMutex = xSemaphoreCreateMutex();
}

void LedController::initialize(LedInfo ledInfo) {
    xSemaphoreTake(mMutex, portMAX_DELAY);
    mLedInfo = ledInfo;
    xSemaphoreGive(mMutex);

    led_strip_config_t ledConfig = {};
    ledConfig.strip_gpio_num = mLedPin;
    ledConfig.max_leds = kLedCount;
    ledConfig.led_model = LED_MODEL_WS2812;
    ledConfig.color_component_format =
        LED_STRIP_COLOR_COMPONENT_FMT_GRB;   // WS2812 uses GRB order

    led_strip_rmt_config_t rmtConfig = {};
    rmtConfig.clk_src = RMT_CLK_SRC_DEFAULT;
    rmtConfig.resolution_hz = 10 * 1000 * 1000;

    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(&ledConfig, &rmtConfig, &mLedHandle));
}

void LedController::update() {
    if (mDirection) {
        if (mCounter < kMaxBrightness) {
            mCounter++;
        } else {
            mDirection = false;
        }
    } else {
        if (mCounter == 0) {
            mDirection = true;
        } else {
            mCounter--;
        }
    }

    xSemaphoreTake(mMutex, portMAX_DELAY);
    switch (mLedInfo.getState()) {
    case LedState::Off: {
        ESP_ERROR_CHECK(led_strip_clear(mLedHandle));
        ESP_ERROR_CHECK(led_strip_refresh(mLedHandle));
        break;
    }
    case LedState::On: {
        ESP_ERROR_CHECK(led_strip_set_pixel(mLedHandle, 0, mLedInfo.getRed(),
                                            mLedInfo.getGreen(),
                                            mLedInfo.getBlue()));
        ESP_ERROR_CHECK(led_strip_refresh(mLedHandle));
        break;
    }
    case LedState::Fade: {
        // An 8-bit gamma-correction table for achieving a better looking
        // brightness perception
        // Code borrowed from:
        // https://cdn-learn.adafruit.com/downloads/pdf/led-tricks-gamma-correction.pdf
        const uint8_t gamma8[] = {
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,
            1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,
            2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   4,   4,
            4,   4,   5,   5,   5,   5,   5,   6,   6,   6,   6,   7,   7,
            7,   8,   8,   8,   9,   9,   9,   10,  10,  10,  11,  11,  11,
            12,  12,  13,  13,  13,  14,  14,  15,  15,  16,  16,  17,  17,
            18,  18,  19,  19,  20,  20,  21,  21,  22,  22,  23,  24,  24,
            25,  25,  26,  27,  27,  28,  29,  29,  30,  31,  31,  32,  33,
            34,  34,  35,  36,  37,  38,  38,  39,  40,  41,  42,  42,  43,
            44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,
            57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  68,  69,  70,
            71,  72,  73,  75,  76,  77,  78,  80,  81,  82,  84,  85,  86,
            88,  89,  90,  92,  93,  94,  96,  97,  99,  100, 102, 103, 105,
            106, 108, 109, 111, 112, 114, 115, 117, 119, 120, 122, 124, 125,
            127, 129, 130, 132, 134, 136, 137, 139, 141, 143, 145, 146, 148,
            150, 152, 154, 156, 158, 160, 162, 164, 166, 168, 170, 172, 174,
            176, 178, 180, 182, 184, 186, 188, 191, 193, 195, 197, 199, 202,
            204, 206, 209, 211, 213, 215, 218, 220, 223, 225, 227, 230, 232,
            235, 237, 240, 242, 245, 247, 250, 252, 255};

        ESP_ERROR_CHECK(led_strip_set_pixel(
            mLedHandle, 0,
            (mLedInfo.getRed() * gamma8[mCounter]) / kMaxBrightness,
            (mLedInfo.getGreen() * gamma8[mCounter]) / kMaxBrightness,
            (mLedInfo.getBlue() * gamma8[mCounter]) / kMaxBrightness));
        ESP_ERROR_CHECK(led_strip_refresh(mLedHandle));

        break;
    }
    case LedState::Pulse: {
        if (mCounter < kPulseTime || mCounter >= kMaxBrightness - kPulseTime) {
            ESP_ERROR_CHECK(
                led_strip_set_pixel(mLedHandle, 0, mLedInfo.getRed(),
                                    mLedInfo.getGreen(), mLedInfo.getBlue()));
        } else {
            ESP_ERROR_CHECK(led_strip_clear(mLedHandle));
        }
        ESP_ERROR_CHECK(led_strip_refresh(mLedHandle));
        break;
    }
    default:
        break;
    }
    xSemaphoreGive(mMutex);
}

void LedController::setLedInfo(const LedInfo& ledInfo) {
    xSemaphoreTake(mMutex, portMAX_DELAY);
    if (mLock) {
        xSemaphoreGive(mMutex);
        return;
    }
    mLedInfo = ledInfo;
    xSemaphoreGive(mMutex);
}

LedInfo LedController::getLedInfo() {
    xSemaphoreTake(mMutex, portMAX_DELAY);
    auto ledInfo = mLedInfo;
    xSemaphoreGive(mMutex);
    return ledInfo;
}

void LedController::lock() {
    xSemaphoreTake(mMutex, portMAX_DELAY);
    mLock = true;
    xSemaphoreGive(mMutex);
}

void LedController::unlock() {
    xSemaphoreTake(mMutex, portMAX_DELAY);
    mLock = false;
    xSemaphoreGive(mMutex);
}