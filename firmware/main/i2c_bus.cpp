/******************************************************************************
 * File:    i2c_bus.cpp
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Object-oriented wrapper around the ESP-IDF I2C master driver
 ******************************************************************************/

#include "i2c_bus.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

I2cBus::I2cBus(i2c_port_t port, gpio_num_t sda, gpio_num_t scl)
    : mPort(port), mSda(sda), mScl(scl), mBus(nullptr) {}

I2cBus::~I2cBus() {
    if (mBus) {
        i2c_del_master_bus(mBus);
        mBus = nullptr;
    }
}

esp_err_t I2cBus::initialize() {
    i2c_master_bus_config_t config = {};
    config.i2c_port = mPort;
    config.sda_io_num = mSda;
    config.scl_io_num = mScl;
    config.clk_source = I2C_CLK_SRC_DEFAULT;
    config.glitch_ignore_cnt = 7;
    config.flags = {.enable_internal_pullup = true};
    return i2c_new_master_bus(&config, &mBus);
}

esp_err_t I2cBus::addDevice(uint8_t address, uint32_t freq,
                            i2c_master_dev_handle_t* dev) {
    i2c_device_config_t dev_cfg = {.device_address = address,
                                   .scl_speed_hz = freq};
    return i2c_master_bus_add_device(mBus, &dev_cfg, dev);
}

esp_err_t I2cBus::write(i2c_master_dev_handle_t dev, const uint8_t* data,
                        size_t len, uint32_t timeout_ms) {
    return i2c_master_transmit(dev, data, len, pdMS_TO_TICKS(timeout_ms));
}

esp_err_t I2cBus::read(i2c_master_dev_handle_t dev, const uint8_t* reg,
                       size_t reg_len, uint8_t* data, size_t data_len,
                       uint32_t timeout_ms) {
    // If reg_len == 0, just read
    if (reg_len > 0) {
        esp_err_t ret =
            i2c_master_transmit(dev, reg, reg_len, pdMS_TO_TICKS(timeout_ms));
        if (ret != ESP_OK)
            return ret;
    }
    return i2c_master_receive(dev, data, data_len, pdMS_TO_TICKS(timeout_ms));
}