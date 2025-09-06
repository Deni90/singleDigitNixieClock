/******************************************************************************
 * File:    bcd_2_decimal_decoder.h
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Header for BDC to decimal decoder
 ******************************************************************************/

#ifndef bcd_2_decimal_decoder_h
#define bcd_2_decimal_decoder_h

#include <inttypes.h>

#include "driver/gpio.h"

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
  public:
    /**
     * @brief Construct a new BCD2DecimalDecoder object
     *
     * @param pinA BCD's input A pin
     * @param pinB BCD's input B pin
     * @param pinC BCD's input C pin
     * @param pinD BCD's input D pin
     */
    BCD2DecimalDecoder(gpio_num_t pinA, gpio_num_t pinB, gpio_num_t pinC,
                       gpio_num_t pinD);

    /**
     * @brief Initialize decoder and clear all outputs
     */
    void initialize();

    /**
     * @brief Decode a value
     *
     * Decoder decodes values from 0 to 9. If value is higher than 9, then the
     * output is clipped to NONE.
     *
     * @param value value
     */
    void decode(uint8_t value);

  private:
    gpio_num_t mPinA;
    gpio_num_t mPinB;
    gpio_num_t mPinC;
    gpio_num_t mPinD;
};

#endif   // bcd_2_decimal_decoder_h