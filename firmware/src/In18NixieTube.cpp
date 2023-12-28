#include "In18NixieTube.h"

In18NixieTube::In18NixieTube(BCD2DecimalDecoder& decoder) : decoder(decoder) {}

void
In18NixieTube::Initialize() {
    decoder.Initialize();
}

void
In18NixieTube::ShowDigit(uint8_t digit) {
    // It is posible to show only digits between 0 and 9
    if (digit > 9) {
        return;
    }
    // The part that I used in EAGLE when creating my PCB layout was incorrect
    // (at least for my tubes). My tubes seem to have a different pinout.
    uint8_t truth_table[] = {1, 0, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    decoder.Decode(truth_table[digit]);
}

void
In18NixieTube::HideDigit() {
    decoder.Decode(NONE);
}