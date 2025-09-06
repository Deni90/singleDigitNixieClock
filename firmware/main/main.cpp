/******************************************************************************
 * File:    main.cpp
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Implements main entry point of the project
 ******************************************************************************/

#include "esp_log.h"

#include "nixie_clock.h"

static const char* kTag = "main";
static NixieClock gNixieClock;

extern "C" void app_main(void) {
    ESP_LOGI(kTag, "Single Digit Nixie Clock");
    gNixieClock.initialize();
}