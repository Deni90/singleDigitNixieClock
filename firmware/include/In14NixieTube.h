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
    BCD2DecimalDecoder& decoder;

  public:
    /**
     * @brief Construct a new In 14 Nixie Tube object
     *
     * @param decoder reference to BCD2DecimalDecoder object
     */
    In14NixieTube(BCD2DecimalDecoder& decoder);

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