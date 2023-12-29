#pragma once

#include "ESPAsyncWebServer.h"
#include <inttypes.h>

#include "ClockInterface.h"

class WebServer {
  private:
    AsyncWebServer server;
    ClockInterface& callback;

    void HandleBacklight(AsyncWebServerRequest* request);
    void HandleSetBacklightState(AsyncWebServerRequest* request);
    void HandleSetBacklightColor(AsyncWebServerRequest* request);
    void HandleSetCurrentTime(AsyncWebServerRequest* request);

  public:
    WebServer(int port, ClockInterface& callback);
    void Initialize();
};