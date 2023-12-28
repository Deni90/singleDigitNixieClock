#pragma once

#include <inttypes.h>
#include "BCD2DecimalDecoder.h"

class In18NixieTube
{
private:
    BCD2DecimalDecoder &decoder;

public:
    In18NixieTube(BCD2DecimalDecoder &decoder);
    void Initialize();
    void ShowDigit(uint8_t digit);
    void HideDigit();
};