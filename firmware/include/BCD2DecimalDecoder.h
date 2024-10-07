#pragma once

#include <inttypes.h>

#define NONE 0x0f   // ecodes to all low outputs

/**
 * @brief Represents a BCD to decimal decoder
 *
 * Truth table:
 * D C B A | 0 1 2 3 4 5 6 7 8 9
 * -----------------------------
 * 0 0 0 0 | 1 0 0 0 0 0 0 0 0 0
 * 0 0 0 1 | 0 1 0 0 0 0 0 0 0 0
 * 0 0 1 0 | 0 0 1 0 0 0 0 0 0 0
 * 0 0 1 1 | 0 0 0 1 0 0 0 0 0 0
 * 0 1 0 0 | 0 0 0 0 1 0 0 0 0 0
 * 0 1 0 1 | 0 0 0 0 0 1 0 0 0 0
 * 0 1 1 0 | 0 0 0 0 0 0 1 0 0 0
 * 0 1 1 1 | 0 0 0 0 0 0 0 1 0 0
 * 1 0 0 0 | 0 0 0 0 0 0 0 0 1 0
 * 1 0 0 1 | 0 0 0 0 0 0 0 0 0 1
 * 1 0 1 0 | 0 0 0 0 0 0 0 0 0 0
 * 1 0 1 1 | 0 0 0 0 0 0 0 0 0 0
 * 1 1 0 0 | 0 0 0 0 0 0 0 0 0 0
 * 1 1 0 1 | 0 0 0 0 0 0 0 0 0 0
 * 1 1 1 0 | 0 0 0 0 0 0 0 0 0 0
 * 1 1 1 1 | 0 0 0 0 0 0 0 0 0 0
 *
 * 1 = high level  0 = low level
 */
class BCD2DecimalDecoder {
  private:
    uint8_t pinA;
    uint8_t pinB;
    uint8_t pinC;
    uint8_t pinD;

  public:
    /**
     * @brief Construct a new BCD2DecimalDecoder object
     *
     * @param pinA BCD's input A pin
     * @param pinB BCD's input B pin
     * @param pinC BCD's input C pin
     * @param pinD BCD's input D pin
     */
    BCD2DecimalDecoder(uint8_t pinA, uint8_t pinB, uint8_t pinC, uint8_t pinD);

    /**
     * @brief Initialize decoder and clear all outputs
     */
    void Initialize();

    /**
     * @brief Decode a value
     *
     * Decoder decodes values from 0 to 9. If value is higher than 9, then the
     * output is clipped to NONE.
     *
     * @param value value
     */
    void Decode(uint8_t value);
};