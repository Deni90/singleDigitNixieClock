/******************************************************************************
 * File:    ds3231.h
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Declarations for the DS3231 real-time clock driver.
 ******************************************************************************/

#ifndef ds3231_h
#define ds3231_h

#include "i2c_bus.h"

#include <ctime>

class Ds3231 {
  public:
    /**
     * @brief Construct the DS3231 driver using a shared I2C bus.
     * @param i2c Reference to an initialized I2C object.
     */
    Ds3231(I2cBus& bus);

    /**
     * @brief Initialize the module
     */
    void initialize();

    /**
     * @brief Read current time and date from the RTC.
     * @param[out] timeinfo Pointer to a struct tm to receive the data.
     * @return true on success, false otherwise.
     */
    bool getTime(struct tm* timeinfo);

    /**
     * @brief Set the RTC to the supplied time and date.
     * @param[in] timeinfo Pointer to struct tm containing the desired time.
     * @return true on success, false otherwise.
     */
    bool setTime(const struct tm* timeinfo);

  private:
    uint8_t bcd2dec(uint8_t val);
    uint8_t dec2bcd(uint8_t val);

    I2cBus& mBus;
    i2c_master_dev_handle_t mDevHandle;
};

#endif   // ds3231_h