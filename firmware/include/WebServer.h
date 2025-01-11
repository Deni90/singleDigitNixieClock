#pragma once

#include "ESPAsyncWebServer.h"
#include <inttypes.h>

#include "ClockInterface.h"

/**
 * @brief A wrapper around an AsyncWebServer
 */
class WebServer {
  private:
    AsyncWebServer server;
    ClockInterface& callback;

    void HandleGetLedInfo(AsyncWebServerRequest* request);
    void HandleSetLedInfo(AsyncWebServerRequest *request, JsonVariant &json);
    void HandleSetCurrentTime(AsyncWebServerRequest *request, JsonVariant &json);
    void HandleGetSleepInfo(AsyncWebServerRequest* request);
    void HandleSetSleepInfo(AsyncWebServerRequest *request, JsonVariant &json);
    void HandleGetWifiInfo(AsyncWebServerRequest* request);
    void HandleSetWifiInfo(AsyncWebServerRequest *request, JsonVariant &json);
    void HandleGetTimeInfo(AsyncWebServerRequest* request);
    void HandleSetTimeInfo(AsyncWebServerRequest *request, JsonVariant &json);

  public:
    /**
     * @brief Construct a new Web Server object
     *
     * @param port port
     * @param callback clock interface object
     */
    WebServer(int port, ClockInterface& callback);

    /**
     * @brief Initialize web server
     *
     * Initialize web server paths and load resources
     */
    void Initialize();
};