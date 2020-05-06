#include "BCD2DecimalDecoder.h"
#include  <Arduino.h>

BCD2DecimalDecoder::BCD2DecimalDecoder( uint8_t pinA_, uint8_t pinB_, uint8_t pinC_, uint8_t pinD_ )
: pinA(pinA_)
, pinB(pinB_)
, pinC(pinC_)
, pinD(pinD_)
{
}

void BCD2DecimalDecoder::Initialize()
{
    pinMode(pinA, OUTPUT);
    pinMode(pinB, OUTPUT);
    pinMode(pinC, OUTPUT);
    pinMode(pinD, OUTPUT);

    //clear all outputs
    digitalWrite(pinA, HIGH);
    digitalWrite(pinB, HIGH);
    digitalWrite(pinC, HIGH);
    digitalWrite(pinD, HIGH);
}

void BCD2DecimalDecoder::Decode(uint8_t value_)
{
    if(value_ > 0x0f)
    {
        value_ = 0x0f; //clip value to 00001111
    }

    digitalWrite(pinA, value_ & (1 << 0));
    digitalWrite(pinB, value_ & (1 << 1));
    digitalWrite(pinC, value_ & (1 << 2));
    digitalWrite(pinD, value_ & (1 << 3));
}