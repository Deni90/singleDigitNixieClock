/******************************************************************************
 * File:    wifi_manager.h
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Declaration of a class used for managing wifi
 ******************************************************************************/

#ifndef wifi_manager_h
#define wifi_manager_h

#include <functional>
#include <string>

#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "wifi_info.h"

class WifiManager {
  public:
    /**
     * @brief Enumeration representing modes of Wifi operation
     */
    enum class Mode { None, Sta, Ap };

    /**
     * @brief Constructor
     */
    WifiManager();

    /**
     * @brief Initializer function
     */
    void initialize();

    /**
     * @brief Connect to a wifi network
     * @param[in] wifiInfo Wifi config
     * @return Status. True if connected, otherwise false.
     */
    bool connectSta(const WifiInfo& wifiInfo);

    /**
     * @brief Start wifi as access point
     * @param[in] wifiInfo Wifi config
     */
    void startAp(const WifiInfo& wifiInfo);

    /**
     * @brief Get mode of the wifi manager.
     * @return mode
     */
    Mode getMode() const;

  private:
    static void eventHandlerStatic(void* arg, esp_event_base_t eventBase,
                                   int32_t eventId, void* eventData);
    void eventHandler(esp_event_base_t eventBase, int32_t eventId,
                      void* eventData);
    Mode mMode;
    EventGroupHandle_t mWifiEventGroup;
};

#endif   // wifi_manager_h