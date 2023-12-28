#pragma once

#include "LedInfo.h"

class ConfigStore
{
public:
    static void SaveLedConfiguration(const LedInfo &ledInfo);
    static void LoadLedConfiguration(LedInfo &ledInfo);
};