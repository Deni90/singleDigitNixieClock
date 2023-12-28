#pragma once

#include <ESP8266WebServer.h>
#include <inttypes.h>

#include "ClockInterface.h"

class WebServer {
  private:
    ESP8266WebServer* webServer;
    ClockInterface* callback;
    WebServer();
    ~WebServer();
    WebServer(const WebServer&) = delete;
    WebServer& operator=(const WebServer&) = delete;

    bool LoadFromLittleFS(String path);
    void HandleRoot();
    void HandleWebRequests();

    void HandleBacklight();
    void HandleSetBacklightState();
    void HandleSetBacklightColor();

    void HandleSetCurrentTime();

  public:
    static WebServer& Instance();
    void Initialize(int port, ClockInterface* callback);
    void Handle();
};