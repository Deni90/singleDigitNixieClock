/******************************************************************************
 * File:    in14_nixie_tube.h
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Declaration for the IN-14 Nixie tube adapter class
 ******************************************************************************/

#ifndef in14_nixie_tube_h
#define in14_nixie_tube_h

#include <inttypes.h>

#include "bcd_2_decimal_decoder.h"
#include "driver/gpio.h"

/**
 * @brief Represents an adapted BCD to decimal decoder
 *
 * This class is fixing the pinout of the BCD's output pins to match the pinout
 * of the IN-14 nixie tube.
 */
class In14NixieTube {
  public:
    /**
     * @brief Construct a new In 14 Nixie Tube object
     *
     * @param pinA BCD's input A pin
     * @param pinB BCD's input B pin
     * @param pinC BCD's input C pin
     * @param pinD BCD's input D pin
     */
    In14NixieTube(gpio_num_t pinA, gpio_num_t pinB, gpio_num_t pinC,
                  gpio_num_t pinD);

    /**
     * @brief Initialize the adapter
     */
    void initialize();

    /**
     * @brief Show digit
     *
     * @param digit A number between 0 and 9
     */
    void showDigit(uint8_t digit);

    /**
     * @brief Hide digit
     *
     */
    void hideDigit();

  private:
    BCD2DecimalDecoder mDecoder;
};

#endif   // in14_nixie_tube_h