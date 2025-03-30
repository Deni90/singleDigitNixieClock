DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
ESP_ROOT = ${DIR}/thirdParty/Arduino/
CHIP = esp8266
BOARD = nodemcuv2
SKETCH = $(DIR)/src/main.cpp
BUILD_DIR = $(DIR)/build
LIBS =	\
		${DIR}/include \
		${DIR}/src/* \
		${DIR}/thirdParty/Adafruit_NeoPixel \
		${DIR}/thirdParty/ArduinoJson/src \
		${DIR}/thirdParty/base64_arduino \
		${DIR}/thirdParty/ESPAsyncTCP \
		${DIR}/thirdParty/ESPAsyncWebServer \
		${DIR}/thirdParty/NTPClient \
		${DIR}/thirdParty/Rtc
FS_TYPE = littlefs
FS_DIR = ${DIR}/data

include $(DIR)/thirdParty/makeEspArduino/makeEspArduino.mk