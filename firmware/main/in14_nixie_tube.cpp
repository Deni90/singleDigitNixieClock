/******************************************************************************
 * File:    in14_nixie_tube.cpp
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Implements In14NixieTube adapter for controlling IN-14 nixie tube
 ******************************************************************************/

#include "in14_nixie_tube.h"

In14NixieTube::In14NixieTube(gpio_num_t pinA, gpio_num_t pinB, gpio_num_t pinC,
                             gpio_num_t pinD)
    : mDecoder(pinA, pinB, pinC, pinD) {}

void In14NixieTube::initialize() { mDecoder.initialize(); }

void In14NixieTube::showDigit(uint8_t digit) {
    // It is posible to show only digits between 0 and 9
    if (digit > 9) {
        return;
    }
    // The part that I used in EAGLE when creating my PCB layout was incorrect
    // (at least for my tubes). My tubes seem to have a different pinout.
    const uint8_t truthTable[] = {1, 0, 9, 8, 7, 6, 5, 4, 3, 2};
    mDecoder.decode(truthTable[digit]);
}

void In14NixieTube::hideDigit() { mDecoder.decode(NONE); }