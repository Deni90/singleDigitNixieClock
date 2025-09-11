###############################################################################
# Project:   SingleDigitNixieClock
# File:      CMakeLists.txt
# Author:    Daniel Knezevic
# Year:      2025
# Brief:     Build configuration for the project.
###############################################################################

idf_component_register(
    SRCS
        bcd_2_decimal_decoder.cpp
        config_store.cpp
        in14_nixie_tube.cpp
        led_controller.cpp
        led_info.cpp
        nixie_clock.cpp
        main.cpp
        nixie_clock.cpp
        sleep_info.cpp
        time_info.cpp
        web_server.cpp
        wifi_info.cpp
        wifi_manager.cpp
    PRIV_REQUIRES
        dns_server
        driver
        esp_driver_gpio
        esp_driver_rmt
        esp_event
        esp_http_server
        esp_wifi
        json
        mbedtls
        nvs_flash
    INCLUDE_DIRS
        include
)

# Note: you must have a partition named the first argument (here it's "littlefs")
# in your partition table csv file.
littlefs_create_partition_image(littlefs ../flash_data FLASH_IN_PROJECT)
