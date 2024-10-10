#include <ArduinoJson.h>
#include <LittleFS.h>

#include "WebServer.h"

using namespace std::placeholders;

namespace {
constexpr int HTTP_200_OK = 200;
constexpr int HTTP_400_BAD_REQUEST = 400;
constexpr int HTTP_500_INTERNAL_SERVER_ERROR = 500;
}   // namespace

WebServer::WebServer(int port, ClockInterface& callback)
    : server(port), callback(callback) {}

void WebServer::Initialize() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/index.html");
    });
    server.onNotFound([](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/index.html");
    });
    server.on("/css/my-custom-theme.min.css", HTTP_GET,
              [](AsyncWebServerRequest* request) {
                  request->send(LittleFS, "/css/my-custom-theme.min.css",
                                "text/css");
              });
    server.on("/css/jquery.mobile.struc.min.css", HTTP_GET,
              [](AsyncWebServerRequest* request) {
                  request->send(LittleFS, "/css/jquery.mobile.struc.min.css",
                                "text/css");
              });
    server.on(
        "/jquery-2.2.4.min.js", HTTP_GET, [](AsyncWebServerRequest* request) {
            request->send(LittleFS, "/jquery-2.2.4.min.js", "text/javascript");
        });
    server.on("/jquery.mobile-1.4.5.min.js", HTTP_GET,
              [](AsyncWebServerRequest* request) {
                  request->send(LittleFS, "/jquery.mobile-1.4.5.min.js",
                                "text/javascript");
              });
    server.on("/server.js", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/server.js", "text/javascript");
    });
    server.on("/css/images/ajax-loader.gif", HTTP_GET,
              [](AsyncWebServerRequest* request) {
                  request->send(LittleFS, "/css/images/ajax-loader.gif",
                                "text/image");
              });
    server.on("/backlight", HTTP_GET, [&](AsyncWebServerRequest* request) {
        this->HandleGetLedInfo(request);
    });
    server.on("/backlight", HTTP_POST, [&](AsyncWebServerRequest* request) {
        this->HandleSetLedInfo(request);
    });
    server.on("/clock/time", HTTP_POST, [&](AsyncWebServerRequest* request) {
        this->HandleSetCurrentTime(request);
    });
    server.begin();
}

void WebServer::HandleGetLedInfo(AsyncWebServerRequest* request) {
    LedInfo li = callback.OnGetLedInfo();

    StaticJsonDocument<200> doc;
    char messageBuffer[200];

    doc["state"] = static_cast<uint8_t>(li.GetState());
    doc["R"] = li.GetR();
    doc["G"] = li.GetG();
    doc["B"] = li.GetB();

    serializeJsonPretty(doc, messageBuffer);

    request->send(HTTP_200_OK, "application/json", messageBuffer);
}

void WebServer::HandleSetLedInfo(AsyncWebServerRequest* request) {
    if (request->hasArg("R") && request->hasArg("G") && request->hasArg("B") &&
        request->hasArg("state")) {
        uint8_t r, g, b;
        r = request->arg("R").toInt();
        g = request->arg("G").toInt();
        b = request->arg("B").toInt();
        uint8_t state = request->arg("state").toInt();
        if (state >= static_cast<uint8_t>(LedState::MAX)) {
            request->send(HTTP_400_BAD_REQUEST,
                          "Error HandleSetLedInfo: Invalid state");
            return;
        }
        LedInfo li(r, g, b, static_cast<LedState>(state));
        callback.OnSetLedInfo(li);
        request->send(HTTP_200_OK, "");
    } else {
        request->send(HTTP_400_BAD_REQUEST,
                      "Error HandleSetLedInfo: missing argument(s)!");
    }
}

void WebServer::HandleSetCurrentTime(AsyncWebServerRequest* request) {
    if (request->hasArg("year") && request->hasArg("month") &&
        request->hasArg("day") && request->hasArg("hour") &&
        request->hasArg("minute") && request->hasArg("second")) {
        uint16_t year;
        uint8_t month, day, hour, minute, second;
        year = request->arg("year").toInt();
        month = request->arg("month").toInt();
        day = request->arg("day").toInt();
        hour = request->arg("hour").toInt();
        minute = request->arg("minute").toInt();
        second = request->arg("second").toInt();
        callback.OnSetCurrentTime(year, month, day, hour, minute, second);
        request->send(HTTP_200_OK, "");
    } else {
        request->send(HTTP_400_BAD_REQUEST,
                      "Error HandleSetCurrentTime: missing argument(s)!");
    }
}