#pragma once

#include <inttypes.h>

enum class LedState
{
    OFF = 0,
    ON,
    FADE,
    MAX
};

class LedInfo
{
private:
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
    LedState state;
public:
    LedInfo();
    LedInfo(uint8_t r, uint8_t g, uint8_t b, uint8_t a, LedState state);
    ~LedInfo() = default;
    LedInfo(const LedInfo& ledInfo) = default;
    uint8_t GetR() const;
    void SetR(const uint8_t value);
    uint8_t GetG() const;
    void SetG(const uint8_t value);
    uint8_t GetB() const;
    void SetB(const uint8_t value);
    uint8_t GetA() const;
    void SetA(const uint8_t value);
    LedState GetState() const;
    void SetState(const LedState state);
    void SetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
};