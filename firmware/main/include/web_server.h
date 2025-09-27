/******************************************************************************
 * File:    web_server.h
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Declaration for the HTTP web server.
 ******************************************************************************/

#ifndef web_server_h
#define web_server_h

#include <inttypes.h>

#include "esp_http_server.h"

#include "clock_iface.h"

/**
 * @brief Class representing HTTP web server
 */
class WebServer {
  public:
    /**
     * @brief Construct a new Web Server object
     *
     * @param callback clock interface object
     */
    WebServer(IClock& callback);

    /**
     * @brief Initialize web server
     *
     * Initialize web server paths and load resources
     */
    void initialize();

  private:
    static esp_err_t resourcehandler(httpd_req_t* req);
    static esp_err_t handleGetLedInfo(httpd_req_t* req);
    static esp_err_t handleSetLedInfo(httpd_req_t* req);
    static esp_err_t handleGetSleepInfo(httpd_req_t* req);
    static esp_err_t handleSetSleepInfo(httpd_req_t* req);
    static esp_err_t handleGetTimeInfo(httpd_req_t* req);
    static esp_err_t handleSetTimeInfo(httpd_req_t* req);
    static esp_err_t handleGetWifiInfo(httpd_req_t* req);
    static esp_err_t handleSetWifiInfo(httpd_req_t* req);

    IClock& mCallback;
};

#endif   // web_server_h