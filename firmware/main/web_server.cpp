/******************************************************************************
 * File:    web_server.cpp
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Implements WebServer class
 ******************************************************************************/

#include "web_server.h"

#include <fstream>
#include <iostream>

#include "cJSON.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_vfs.h"

#include "wifi_info.h"

static const char* kTag = "web_server";
static char gScratch[10240];

WebServer::WebServer(IClock& callback) : mCallback(callback) {}

void WebServer::initialize() {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 9;
    config.uri_match_fn = httpd_uri_match_wildcard;

    if (httpd_start(&server, &config) != ESP_OK) {
        return;
    }

    httpd_uri_t ledInfoGetUri = {.uri = "/api/v1/led/led_info",
                                 .method = HTTP_GET,
                                 .handler = handleGetLedInfo,
                                 .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &ledInfoGetUri);

    httpd_uri_t ledInfoPostUri = {.uri = "/api/v1/led/led_info",
                                  .method = HTTP_POST,
                                  .handler = handleSetLedInfo,
                                  .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &ledInfoPostUri);

    httpd_uri_t sleepInfoGetUri = {.uri = "/api/v1/clock/sleep_info",
                                   .method = HTTP_GET,
                                   .handler = handleGetSleepInfo,
                                   .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &sleepInfoGetUri);

    httpd_uri_t sleepInfoPostUri = {.uri = "/api/v1/clock/sleep_info",
                                    .method = HTTP_POST,
                                    .handler = handleSetSleepInfo,
                                    .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &sleepInfoPostUri);

    httpd_uri_t timeInfoGetUri = {.uri = "/api/v1/clock/time_info",
                                  .method = HTTP_GET,
                                  .handler = handleGetTimeInfo,
                                  .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &timeInfoGetUri);

    httpd_uri_t timeInfoPostUri = {.uri = "/api/v1/clock/time_info",
                                   .method = HTTP_POST,
                                   .handler = handleSetTimeInfo,
                                   .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &timeInfoPostUri);

    httpd_uri_t wifiInfoGetUri = {.uri = "/api/v1/wifi/wifi_info",
                                  .method = HTTP_GET,
                                  .handler = handleGetWifiInfo,
                                  .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &wifiInfoGetUri);

    httpd_uri_t wifiInfoPostUri = {.uri = "/api/v1/wifi/wifi_info",
                                   .method = HTTP_POST,
                                   .handler = handleSetWifiInfo,
                                   .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &wifiInfoPostUri);

    httpd_uri_t commonGetUri = {.uri = "/*",
                                .method = HTTP_GET,
                                .handler = resourcehandler,
                                .user_ctx = nullptr};
    httpd_register_uri_handler(server, &commonGetUri);
}

esp_err_t WebServer::resourcehandler(httpd_req_t* req) {
    std::string filepath;
    std::string uri = req->uri;
    if (uri == "/") {
        filepath = "/littlefs/frontend/index.html";
        httpd_resp_set_type(req, "text/html");
    } else if (uri == "/style.css") {
        filepath = "/littlefs/frontend/style.css";
        httpd_resp_set_type(req, "text/css");
    } else if (uri == "/server.js") {
        filepath = "/littlefs/frontend/server.js";
        httpd_resp_set_type(req, "application/javascript");
    } else if (uri == "/zones.json") {
        filepath = "/littlefs/frontend/zones.json";
        httpd_resp_set_type(req, "application/json");
    } else {
        filepath = "/littlefs/frontend/index.html";
        httpd_resp_set_type(req, "text/html");
    }
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        ESP_LOGE(kTag, "Resourcehandler - Failed to open file : %s",
                 filepath.c_str());
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to read existing file");
        return ESP_FAIL;
    }
    while (file.read(gScratch, sizeof(gScratch)) || file.gcount() > 0) {
        size_t bytes_read = file.gcount();
        // Send the buffer contents as HTTP response chunk
        if (httpd_resp_send_chunk(req, gScratch, bytes_read) != ESP_OK) {
            ESP_LOGE(kTag, "Resourcehandler - File sending failed!");
            // Abort sending file
            httpd_resp_sendstr_chunk(req, NULL);
            // Respond with 500 Internal Server Error
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                "Failed to send file");
            return ESP_FAIL;
        }
    }
    ESP_LOGI(kTag, "Resourcehandler - Sending file '%s' completed",
             filepath.c_str());
    // Respond with an empty chunk to signal HTTP response completion
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t WebServer::handleGetLedInfo(httpd_req_t* req) {
    IClock* callback = static_cast<IClock*>(req->user_ctx);
    auto maybeWLedInfo = callback->onGetLedInfo();
    if (!maybeWLedInfo.has_value()) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to get LedInfo");
        return ESP_FAIL;
    }
    auto ledInfo = maybeWLedInfo.value();
    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "state",
                            ledStateToString(ledInfo.getState()));
    cJSON_AddNumberToObject(root, "R", ledInfo.getRed());
    cJSON_AddNumberToObject(root, "G", ledInfo.getGreen());
    cJSON_AddNumberToObject(root, "B", ledInfo.getBlue());
    char* jsonStr = cJSON_Print(root);
    httpd_resp_sendstr(req, jsonStr);
    free(static_cast<void*>(jsonStr));
    cJSON_Delete(root);
    return ESP_OK;
}

esp_err_t WebServer::handleSetLedInfo(httpd_req_t* req) {
    IClock* callback = static_cast<IClock*>(req->user_ctx);
    int cur_len = 0;
    int received = 0;
    if (req->content_len >= sizeof(gScratch)) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "content too long");
        return ESP_FAIL;
    }
    while (cur_len < req->content_len) {
        received = httpd_req_recv(req, gScratch + cur_len, req->content_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    gScratch[req->content_len] = '\0';
    cJSON* root = cJSON_Parse(gScratch);
    LedInfo ledInfo;
    // ledInfo.setState(cJSON_GetObjectItem(root, "hostname")->valuestring);
    ledInfo.setRed(cJSON_GetObjectItem(root, "R")->valueint);
    ledInfo.setGreen(cJSON_GetObjectItem(root, "G")->valueint);
    ledInfo.setBlue(cJSON_GetObjectItem(root, "B")->valueint);
    std::string state =
        cJSON_GetObjectItemCaseSensitive(root, "state")->valuestring;
    if (state == "on") {
        ledInfo.setState(LedState::On);
    } else if (state == "off") {
        ledInfo.setState(LedState::Off);
    } else if (state == "fade") {
        ledInfo.setState(LedState::Fade);
    } else if (state == "pulse") {
        ledInfo.setState(LedState::Pulse);
    } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Unknown state");
        return ESP_FAIL;
    }
    callback->onSetLedInfo(ledInfo);
    cJSON_Delete(root);
    httpd_resp_sendstr(req, "Post control value successfully");
    return ESP_OK;
}

esp_err_t WebServer::handleGetSleepInfo(httpd_req_t* req) {
    IClock* callback = static_cast<IClock*>(req->user_ctx);
    auto maybeSleepInfo = callback->onGetSleepInfo();
    if (!maybeSleepInfo.has_value()) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to get SleepInfo");
        return ESP_FAIL;
    }
    auto sleepInfo = maybeSleepInfo.value();
    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "sleep_before", sleepInfo.getSleepBefore());
    cJSON_AddNumberToObject(root, "sleep_after", sleepInfo.getSleepAfter());
    char* jsonStr = cJSON_Print(root);
    httpd_resp_sendstr(req, jsonStr);
    free(static_cast<void*>(jsonStr));
    cJSON_Delete(root);
    return ESP_OK;
}

esp_err_t WebServer::handleSetSleepInfo(httpd_req_t* req) {
    IClock* callback = static_cast<IClock*>(req->user_ctx);
    int cur_len = 0;
    int received = 0;
    if (req->content_len >= sizeof(gScratch)) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "content too long");
        return ESP_FAIL;
    }
    while (cur_len < req->content_len) {
        received = httpd_req_recv(req, gScratch + cur_len, req->content_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    gScratch[req->content_len] = '\0';
    cJSON* root = cJSON_Parse(gScratch);
    SleepInfo sleepInfo;
    sleepInfo.setSleepBefore(
        cJSON_GetObjectItem(root, "sleep_before")->valueint);
    sleepInfo.setSleepAfter(cJSON_GetObjectItem(root, "sleep_after")->valueint);
    callback->onSetSleepInfo(sleepInfo);
    cJSON_Delete(root);
    httpd_resp_sendstr(req, "Post control value successfully");
    return ESP_OK;
}

esp_err_t WebServer::handleGetTimeInfo(httpd_req_t* req) {
    IClock* callback = static_cast<IClock*>(req->user_ctx);
    auto maybeTimeInfo = callback->onGetTimeInfo();
    if (!maybeTimeInfo.has_value()) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to get SleepInfo");
        return ESP_FAIL;
    }
    auto timeInfo = maybeTimeInfo.value();
    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "tz_zone", timeInfo.getTzZone().c_str());
    cJSON_AddStringToObject(root, "tz_offset", timeInfo.getTzOffset().c_str());
    cJSON_AddStringToObject(root, "time_format",
                            timeFormatToString(timeInfo.getTimeFormat()));
    char* jsonStr = cJSON_Print(root);
    httpd_resp_sendstr(req, jsonStr);
    free(static_cast<void*>(jsonStr));
    cJSON_Delete(root);
    return ESP_OK;
}

esp_err_t WebServer::handleSetTimeInfo(httpd_req_t* req) {
    IClock* callback = static_cast<IClock*>(req->user_ctx);
    int cur_len = 0;
    int received = 0;
    if (req->content_len >= sizeof(gScratch)) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "content too long");
        return ESP_FAIL;
    }
    while (cur_len < req->content_len) {
        received = httpd_req_recv(req, gScratch + cur_len, req->content_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    gScratch[req->content_len] = '\0';
    cJSON* root = cJSON_Parse(gScratch);
    TimeInfo timeInfo;
    timeInfo.setTzZone(cJSON_GetObjectItem(root, "tz_zone")->valuestring);
    timeInfo.setTzOffset(cJSON_GetObjectItem(root, "tz_offset")->valuestring);
    std::string timeFormat =
        cJSON_GetObjectItemCaseSensitive(root, "time_format")->valuestring;
    if (timeFormat == "12h") {
        timeInfo.setTimeFormat(TimeFormat::Hour12);
    } else if (timeFormat == "24h") {
        timeInfo.setTimeFormat(TimeFormat::Hour24);
    } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Unknown time format");
        return ESP_FAIL;
    }
    callback->onSetTimeInfo(timeInfo);
    cJSON_Delete(root);
    httpd_resp_sendstr(req, "Post control value successfully");
    return ESP_OK;
}

esp_err_t WebServer::handleGetWifiInfo(httpd_req_t* req) {
    IClock* callback = static_cast<IClock*>(req->user_ctx);
    auto maybeWifiInfo = callback->onGetWifiInfo();
    if (!maybeWifiInfo.has_value()) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to get WifiInfo");
        return ESP_FAIL;
    }
    auto wifiInfo = maybeWifiInfo.value();
    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "hostname", wifiInfo.getHostname().c_str());
    cJSON_AddStringToObject(root, "SSID", wifiInfo.getSSID().c_str());
    cJSON_AddStringToObject(root, "password", wifiInfo.getPassword().c_str());
    char* jsonStr = cJSON_Print(root);
    httpd_resp_sendstr(req, jsonStr);
    free(static_cast<void*>(jsonStr));
    cJSON_Delete(root);
    return ESP_OK;
}

esp_err_t WebServer::handleSetWifiInfo(httpd_req_t* req) {
    IClock* callback = static_cast<IClock*>(req->user_ctx);
    int cur_len = 0;
    int received = 0;
    if (req->content_len >= sizeof(gScratch)) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "content too long");
        return ESP_FAIL;
    }
    while (cur_len < req->content_len) {
        received = httpd_req_recv(req, gScratch + cur_len, req->content_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    gScratch[req->content_len] = '\0';
    cJSON* root = cJSON_Parse(gScratch);
    WifiInfo wifiInfo;
    wifiInfo.setHostname(cJSON_GetObjectItem(root, "hostname")->valuestring);
    wifiInfo.setSSID(cJSON_GetObjectItem(root, "SSID")->valuestring);
    wifiInfo.setPassword(cJSON_GetObjectItem(root, "password")->valuestring);
    callback->onSetWifiInfo(wifiInfo);
    cJSON_Delete(root);
    httpd_resp_sendstr(req, "Post control value successfully");
    return ESP_OK;
}