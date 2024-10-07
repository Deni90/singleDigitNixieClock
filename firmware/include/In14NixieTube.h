#pragma once

#include "BCD2DecimalDecoder.h"
#include <inttypes.h>

/**
 * @brief Represents an adapted BCD to decimal decoder
 *
 * This class is fixing the pinout of the BCD's output pins to match the pinout
 * of the IN-14 nixie tube.
 */
class In14NixieTube {
  private:
    BCD2DecimalDecoder decoder;

  public:
    /**
     * @brief Construct a new In 14 Nixie Tube object
     *
     * @param pinA BCD's input A pin
     * @param pinB BCD's input B pin
     * @param pinC BCD's input C pin
     * @param pinD BCD's input D pin
     */
    In14NixieTube(uint8_t pinA, uint8_t pinB, uint8_t pinC, uint8_t pinD);

    /**
     * @brief Initialize the adapter
     */
    void Initialize();

    /**
     * @brief Show digit
     *
     * @param digit A number between 0 and 9
     */
    void ShowDigit(uint8_t digit);

    /**
     * @brief Hide digit
     *
     */
    void HideDigit();
};