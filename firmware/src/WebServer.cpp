#include "AsyncJson.h"
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
        request->send(LittleFS, "/web_server/index.html");
    });
    server.onNotFound([](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/web_server/index.html");
    });
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/web_server/style.css", "text/css");
    });
    server.on("/server.js", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/web_server/server.js", "text/javascript");
    });
    server.on("/backlight", HTTP_GET, [&](AsyncWebServerRequest* request) {
        this->HandleGetLedInfo(request);
    });
    server.addHandler(new AsyncCallbackJsonWebHandler(
        "/backlight", [&](AsyncWebServerRequest* request, JsonVariant& json) {
            this->HandleSetLedInfo(request, json);
        }));
    server.addHandler(new AsyncCallbackJsonWebHandler(
        "/clock/time", [&](AsyncWebServerRequest* request, JsonVariant& json) {
            this->HandleSetCurrentTime(request, json);
        }));
    server.on("/clock/sleep_info", HTTP_GET,
              [&](AsyncWebServerRequest* request) {
                  this->HandleGetSleepInfo(request);
              });
    server.addHandler(new AsyncCallbackJsonWebHandler(
        "/clock/sleep_info",
        [&](AsyncWebServerRequest* request, JsonVariant& json) {
            this->HandleSetSleepInfo(request, json);
        }));
    server.on("/clock/time_info", HTTP_GET,
              [&](AsyncWebServerRequest* request) {
                  this->HandleGetTimeInfo(request);
              });
    server.addHandler(new AsyncCallbackJsonWebHandler(
        "/clock/time_info",
        [&](AsyncWebServerRequest* request, JsonVariant& json) {
            this->HandleSetTimeInfo(request, json);
        }));
    server.on("/wifi", HTTP_GET, [&](AsyncWebServerRequest* request) {
        this->HandleGetWifiInfo(request);
    });
    server.addHandler(new AsyncCallbackJsonWebHandler(
        "/wifi", [&](AsyncWebServerRequest* request, JsonVariant& json) {
            this->HandleSetWifiInfo(request, json);
        }));
    server.begin();
}

void WebServer::HandleGetLedInfo(AsyncWebServerRequest* request) {
    if (!request) {
        return;
    }
    LedInfo li = callback.OnGetLedInfo();
    String messageBuffer;
    serializeJson(li.ToJson(), messageBuffer);
    request->send(HTTP_200_OK, "application/json", messageBuffer);
}

void WebServer::HandleSetLedInfo(AsyncWebServerRequest* request,
                                 JsonVariant& json) {
    if (!request) {
        return;
    }
    if (!json.is<JsonObject>()) {
        request->send(HTTP_400_BAD_REQUEST,
                      "Error HandleSetLedInfo: expecting a JSON object");
        return;
    }
    JsonDocument requestBody = json.as<JsonObject>();
    if (!requestBody.containsKey("R") || !requestBody.containsKey("G") ||
        !requestBody.containsKey("B") || !requestBody.containsKey("state")) {
        request->send(HTTP_400_BAD_REQUEST,
                      "Error HandleSetLedInfo: missing argument(s)!");
        return;
    }
    uint8_t state = requestBody["state"];
    if (state >= static_cast<uint8_t>(LedState::MAX)) {
        request->send(HTTP_400_BAD_REQUEST,
                      "Error HandleSetLedInfo: Invalid state");
        return;
    }
    LedInfo li(requestBody["R"], requestBody["G"], requestBody["B"],
               static_cast<LedState>(state));
    callback.OnSetLedInfo(li);
    request->send(200, "");
}

void WebServer::HandleSetCurrentTime(AsyncWebServerRequest* request,
                                     JsonVariant& json) {
    if (!request) {
        return;
    }
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

void WebServer::HandleGetSleepInfo(AsyncWebServerRequest* request) {
    if (!request) {
        return;
    }
    SleepInfo si = callback.OnGetSleepInfo();
    String messageBuffer;
    serializeJson(si.ToJson(), messageBuffer);
    request->send(HTTP_200_OK, "application/json", messageBuffer);
}

void WebServer::HandleSetSleepInfo(AsyncWebServerRequest* request,
                                   JsonVariant& json) {
    if (!request) {
        return;
    }
    if (!json.is<JsonObject>()) {
        request->send(HTTP_400_BAD_REQUEST,
                      "Error HandleSetSleepInfo: expecting a JSON object");
        return;
    }
    JsonDocument requestBody = json.as<JsonObject>();
    if (!requestBody.containsKey("sleep_before") ||
        !requestBody.containsKey("sleep_after")) {
        request->send(HTTP_400_BAD_REQUEST,
                      "Error HandleSetSleepInfo: missing argument(s)!");
        return;
    }
    SleepInfo si(requestBody["sleep_before"], requestBody["sleep_after"]);
    callback.OnSetSleepInfo(si);
    request->send(HTTP_200_OK, "");
}

void WebServer::HandleGetWifiInfo(AsyncWebServerRequest* request) {
    if (!request) {
        return;
    }
    WifiInfo wi = callback.OnGetWifiInfo();
    String messageBuffer;
    serializeJson(wi.ToJson(), messageBuffer);
    request->send(HTTP_200_OK, "application/json", messageBuffer);
}

void WebServer::HandleSetWifiInfo(AsyncWebServerRequest* request,
                                  JsonVariant& json) {
    if (!request) {
        return;
    }
    if (!json.is<JsonObject>()) {
        request->send(HTTP_400_BAD_REQUEST,
                      "Error HandleSetWifiInfo: expecting a JSON object");
        return;
    }
    JsonDocument requestBody = json.as<JsonObject>();
    if (!requestBody.containsKey("hostname") ||
        !requestBody.containsKey("SSID") ||
        !requestBody.containsKey("password")) {
        request->send(HTTP_400_BAD_REQUEST,
                      "Error HandleSetWifiInfo: missing argument(s)!");
        return;
    }
    WifiInfo wi(requestBody["hostname"], requestBody["SSID"],
                requestBody["password"]);
    request->send(HTTP_200_OK, "");
    callback.OnSetWifiInfo(wi);
}

void WebServer::HandleGetTimeInfo(AsyncWebServerRequest* request) {
    if (!request) {
        return;
    }
    TimeInfo ti = callback.OnGetTimeInfo();
    String messageBuffer;
    serializeJson(ti.ToJson(), messageBuffer);

    request->send(HTTP_200_OK, "application/json", messageBuffer);
}

void WebServer::HandleSetTimeInfo(AsyncWebServerRequest* request,
                                  JsonVariant& json) {
    if (!request) {
        return;
    }
    if (!json.is<JsonObject>()) {
        request->send(HTTP_400_BAD_REQUEST,
                      "Error HandleSetTimeInfo: expecting a JSON object");
        return;
    }
    JsonDocument requestBody = json.as<JsonObject>();
    if (!requestBody.containsKey("offset") ||
        !requestBody.containsKey("isDst")) {
        request->send(HTTP_400_BAD_REQUEST,
                      "Error HandleSetTimeInfo: missing argument(s)!");
        return;
    }
    int offset = requestBody["offset"];
    uint8_t isDst = requestBody["isDst"];
    TimeInfo ti(offset, isDst);
    callback.OnSetTimeInfo(ti);
    request->send(HTTP_200_OK, "");
}