/******************************************************************************
 * File:    led_info.cpp
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Implements LedInfo data class
 ******************************************************************************/

#include "led_info.h"

LedInfo::LedInfo() : mRed(0), mGreen(0), mBlue(0), mState(LedState::Off) {}

LedInfo::LedInfo(uint8_t r, uint8_t g, uint8_t b, LedState state)
    : mRed(r), mGreen(g), mBlue(b), mState(state) {}

uint8_t LedInfo::getRed() const { return mRed; }

void LedInfo::setRed(const uint8_t value) { mRed = value; }

uint8_t LedInfo::getGreen() const { return mGreen; }

void LedInfo::setGreen(const uint8_t value) { mGreen = value; }

uint8_t LedInfo::getBlue() const { return mBlue; }

void LedInfo::setBlue(const uint8_t value) { mBlue = value; }

LedState LedInfo::getState() const { return mState; }

void LedInfo::setState(const LedState state) { mState = state; }

void LedInfo::setColor(uint8_t r, uint8_t g, uint8_t b) {
    mRed = r;
    mGreen = g;
    mBlue = b;
}