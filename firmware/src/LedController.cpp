#include "LedController.h"

namespace {
constexpr uint16_t LED_COUNT = 1;
constexpr uint8_t MAX_BRIGHTNESS = 255;
constexpr uint8_t PULSE_TIME = 10;
}   // namespace

LedController::LedController(uint16_t ledPin)
    : ledDriver(LED_COUNT, ledPin, NEO_GRB + NEO_KHZ800), counter(0),
      direction(true), lock(false) {}

void LedController::Initialize(LedInfo ledInfo) {
    this->ledInfo = ledInfo;
    ledDriver.begin();
    ledDriver.setBrightness(MAX_BRIGHTNESS);
    ledDriver.setPixelColor(
        0, ledDriver.Color(ledInfo.GetR(), ledInfo.GetG(), ledInfo.GetB()));
    ledDriver.show();
}

void LedController::Update() {
    if (direction) {
        if (counter < MAX_BRIGHTNESS) {
            counter++;
        } else {
            direction = false;
        }
    } else {
        if (counter == 0) {
            direction = true;
        } else {
            counter--;
        }
    }

    switch (ledInfo.GetState()) {
    case LedState::OFF: {
        ledDriver.setBrightness(0);
        break;
    }
    case LedState::ON: {
        ledDriver.setBrightness(MAX_BRIGHTNESS);
        uint32_t color =
            ledDriver.Color(ledInfo.GetR(), ledInfo.GetG(), ledInfo.GetB());
        ledDriver.setPixelColor(0, color);
        break;
    }
    case LedState::FADE: {
        ledDriver.setBrightness(ledDriver.gamma8(counter));
        uint32_t color =
            ledDriver.Color(ledInfo.GetR(), ledInfo.GetG(), ledInfo.GetB());
        ledDriver.setPixelColor(0, color);
        break;
    }
    case LedState::PULSE: {
        uint32_t color =
            ledDriver.Color(ledInfo.GetR(), ledInfo.GetG(), ledInfo.GetB());
        if (counter < PULSE_TIME || counter >= MAX_BRIGHTNESS - PULSE_TIME) {
            ledDriver.setBrightness(MAX_BRIGHTNESS);
        } else {
            ledDriver.setBrightness(0);
        }
        ledDriver.setPixelColor(0, color);
        break;
    }
    case LedState::MAX:
    default:
        break;
    }
    ledDriver.show();
}

void LedController::SetLedInfo(const LedInfo& li) {
    if (lock) {
        return;
    }
    ledInfo = li;
}

LedInfo LedController::GetLedInfo() { return ledInfo; }

void LedController::Lock() { lock = true; }

void LedController::Unlock() { lock = false; }