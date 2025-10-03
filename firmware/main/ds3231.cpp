/******************************************************************************
 * File:    ds3231.h
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Definitions for the DS3231 real-time clock driver.
 ******************************************************************************/

#include "ds3231.h"

static constexpr uint8_t kAddr = 0x68;
static constexpr u_int32_t kFreq = 400000;   // Hz
static constexpr uint8_t kTimeReg = 0x00;

Ds3231::Ds3231(I2cBus& bus) : mBus(bus) {}

void Ds3231::initialize() {
    ESP_ERROR_CHECK(mBus.addDevice(kAddr, kFreq, &mDevHandle));
}

bool Ds3231::getTime(struct tm* tm) {
    uint8_t reg = kTimeReg;
    uint8_t buf[7];
    if (mBus.read(mDevHandle, &reg, 1, buf, sizeof(buf)) != ESP_OK) {
        return false;
    }
    tm->tm_sec = bcd2dec(buf[0]);
    tm->tm_min = bcd2dec(buf[1]);
    tm->tm_hour = bcd2dec(buf[2]);
    tm->tm_mday = bcd2dec(buf[4]);
    tm->tm_mon = bcd2dec(buf[5]) - 1;
    tm->tm_year = bcd2dec(buf[6]) + 100;
    return true;
}

bool Ds3231::setTime(const struct tm* tm) {
    uint8_t buf[8];
    buf[0] = kTimeReg;   // register address
    buf[1] = dec2bcd(tm->tm_sec);
    buf[2] = dec2bcd(tm->tm_min);
    buf[3] = dec2bcd(tm->tm_hour);
    buf[4] = 0;   // weekday (not used)
    buf[5] = dec2bcd(tm->tm_mday);
    buf[6] = dec2bcd(tm->tm_mon + 1);
    buf[7] = dec2bcd(tm->tm_year - 100);
    if (mBus.write(mDevHandle, buf, sizeof(buf)) != ESP_OK) {
        return false;
    }
    return true;
}

uint8_t Ds3231::bcd2dec(uint8_t val) { return (val >> 4) * 10 + (val & 0x0F); }

uint8_t Ds3231::dec2bcd(uint8_t val) { return ((val / 10) << 4) | (val % 10); }