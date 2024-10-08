#include "In14NixieTube.h"

In14NixieTube::In14NixieTube(uint8_t pinA, uint8_t pinB, uint8_t pinC,
                             uint8_t pinD)
    : decoder(pinA, pinB, pinC, pinD) {}

void In14NixieTube::Initialize() { decoder.Initialize(); }

void In14NixieTube::ShowDigit(uint8_t digit) {
    // It is posible to show only digits between 0 and 9
    if (digit > 9) {
        return;
    }
    // The part that I used in EAGLE when creating my PCB layout was incorrect
    // (at least for my tubes). My tubes seem to have a different pinout.
    const uint8_t truthTable[] = {1, 0, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    decoder.Decode(truthTable[digit]);
}

void In14NixieTube::HideDigit() { decoder.Decode(NONE); }