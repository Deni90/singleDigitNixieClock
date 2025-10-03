/******************************************************************************
 * File:    i2c_bus.h
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Object-oriented wrapper around the ESP-IDF I2C master driver
 ******************************************************************************/

#ifndef i2c_bus_h
#define i2c_bus_h

#include "driver/i2c_master.h"
#include "esp_err.h"

class I2cBus {
  public:
    /**
     * @brief Construct a new I2cBus instance.
     *
     * @param port I2C hardware port number (e.g., I2C_NUM_0 or I2C_NUM_1).
     * @param sda GPIO pin number used for the SDA line.
     * @param scl GPIO pin number used for the SCL line.
     */
    I2cBus(i2c_port_t port, gpio_num_t sda, gpio_num_t scl);

    /**
     * @brief Destructor for the I2cBus.
     *
     * Automatically deletes the I2C master bus if it was initialized.
     */
    ~I2cBus();

    /**
     * @brief Initialize the I2C master bus.
     *
     * Configures the I2C master bus with internal pull-ups and default clock
     * source. Must be called before adding devices or performing transactions.
     *
     * @return
     *  - ESP_OK on success
     *  - ESP_ERR_INVALID_ARG if parameters are invalid
     *  - ESP_ERR_NO_MEM if allocation fails
     *  - Other error codes from the ESP-IDF I2C driver
     */
    esp_err_t initialize();

    /**
     * @brief Add a device to the I2C bus.
     *
     * Each device can use its own SCL frequency. The returned device handle
     * can be used in subsequent read/write operations.
     *
     * @param address 7-bit I2C device address.
     * @param freq SCL clock frequency in Hz (e.g., 100000 or 400000).
     * @param dev Pointer to a handle that receives the device handle.
     * @return
     *  - ESP_OK on success
     *  - ESP_ERR_INVALID_ARG or ESP_FAIL on failure
     */
    esp_err_t addDevice(uint8_t address, uint32_t freq,
                        i2c_master_dev_handle_t* dev);

    /**
     * @brief Write a data buffer to an I2C device.
     *
     * @param dev Device handle obtained from addDevice().
     * @param data Pointer to data buffer to transmit.
     * @param len Number of bytes to send.
     * @param timeout_ms Timeout for the operation, in milliseconds.
     * @return
     *  - ESP_OK on success
     *  - ESP_FAIL, ESP_ERR_TIMEOUT, or ESP_ERR_INVALID_STATE on failure
     */
    esp_err_t write(i2c_master_dev_handle_t dev, const uint8_t* data,
                    size_t len, uint32_t timeout_ms = 1000);

    /**
     * @brief Read data from an I2C device, optionally after writing a register
     * address.
     *
     * Performs a combined write-read transaction if @p reg_len > 0.
     *
     * @param dev Device handle obtained from addDevice().
     * @param reg Optional pointer to the register address buffer (can be
     * nullptr).
     * @param reg_len Length of the register address buffer in bytes (0 for no
     * write phase).
     * @param data Pointer to buffer for storing received data.
     * @param data_len Number of bytes to read.
     * @param timeout_ms Timeout for the operation, in milliseconds.
     * @return
     *  - ESP_OK on success
     *  - ESP_FAIL, ESP_ERR_TIMEOUT, or ESP_ERR_INVALID_STATE on failure
     */
    esp_err_t read(i2c_master_dev_handle_t dev, const uint8_t* reg,
                   size_t reg_len, uint8_t* data, size_t data_len,
                   uint32_t timeout_ms = 1000);

  private:
    i2c_port_t mPort;
    gpio_num_t mSda;
    gpio_num_t mScl;
    i2c_master_bus_handle_t mBus;
};

#endif   // i2c_bus_h