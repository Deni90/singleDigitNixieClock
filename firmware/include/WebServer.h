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

    void HandleBacklight(AsyncWebServerRequest* request);
    void HandleSetBacklightState(AsyncWebServerRequest* request);
    void HandleSetBacklightColor(AsyncWebServerRequest* request);
    void HandleSetCurrentTime(AsyncWebServerRequest* request);

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