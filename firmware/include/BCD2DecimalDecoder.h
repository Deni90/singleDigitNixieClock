#pragma once

#include <inttypes.h>

#define NONE 0x0f

class BCD2DecimalDecoder {
  private:
    uint8_t pinA;
    uint8_t pinB;
    uint8_t pinC;
    uint8_t pinD;

  public:
    BCD2DecimalDecoder(uint8_t pinA, uint8_t pinB, uint8_t pinC, uint8_t pinD);
    void Initialize();
    void Decode(uint8_t value);
};