#include "BCD2DecimalDecoder.h"
#include <Arduino.h>

BCD2DecimalDecoder::BCD2DecimalDecoder(uint8_t pinA, uint8_t pinB, uint8_t pinC, uint8_t pinD)
    : pinA(pinA), pinB(pinB), pinC(pinC), pinD(pinD)
{
}

void BCD2DecimalDecoder::Initialize()
{
    pinMode(pinA, OUTPUT);
    pinMode(pinB, OUTPUT);
    pinMode(pinC, OUTPUT);
    pinMode(pinD, OUTPUT);

    // clear all outputs
    digitalWrite(pinA, HIGH);
    digitalWrite(pinB, HIGH);
    digitalWrite(pinC, HIGH);
    digitalWrite(pinD, HIGH);
}

void BCD2DecimalDecoder::Decode(uint8_t value)
{
    if (value > NONE)
    {
        value = NONE; // clip value to 00001111
    }

    digitalWrite(pinA, value & (1 << 0));
    digitalWrite(pinB, value & (1 << 1));
    digitalWrite(pinC, value & (1 << 2));
    digitalWrite(pinD, value & (1 << 3));
}