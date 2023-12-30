#pragma once

#include "BCD2DecimalDecoder.h"
#include <inttypes.h>

class In14NixieTube {
  private:
    BCD2DecimalDecoder& decoder;

  public:
    In14NixieTube(BCD2DecimalDecoder& decoder);
    void Initialize();
    void ShowDigit(uint8_t digit);
    void HideDigit();
};