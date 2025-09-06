/******************************************************************************
 * File:    bcd_2_decimal_decoder.cpp
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Implements BDC to decimal decoder class
 ******************************************************************************/

#include "bcd_2_decimal_decoder.h"

#include "driver/gpio.h"

BCD2DecimalDecoder::BCD2DecimalDecoder(gpio_num_t pinA, gpio_num_t pinB,
                                       gpio_num_t pinC, gpio_num_t pinD)
    : mPinA(pinA), mPinB(pinB), mPinC(pinC), mPinD(pinD) {}

void BCD2DecimalDecoder::initialize() {
    gpio_config_t ioConfig;
    ioConfig.intr_type = GPIO_INTR_DISABLE;   // no interrupt
    ioConfig.mode = GPIO_MODE_OUTPUT;         // set as output mode
    ioConfig.pin_bit_mask = (1 << mPinA) |    // bit mask of the pins
                            (1 << mPinB) | (1 << mPinC) | (1 << mPinD);
    ioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;   // disable pull-down
    ioConfig.pull_up_en = GPIO_PULLUP_DISABLE;       // disable pull-up
    gpio_config(&ioConfig);

    // clear all outputs
    gpio_set_level(mPinA, 1);
    gpio_set_level(mPinB, 1);
    gpio_set_level(mPinC, 1);
    gpio_set_level(mPinD, 1);
}

void BCD2DecimalDecoder::decode(uint8_t value) {
    if (value > NONE) {
        value = NONE;   // clip value to 00001111
    }

    gpio_set_level(mPinA, value & (1 << 0));
    gpio_set_level(mPinB, value & (1 << 1));
    gpio_set_level(mPinC, value & (1 << 2));
    gpio_set_level(mPinD, value & (1 << 3));
}