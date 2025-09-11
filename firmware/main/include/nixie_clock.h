#ifndef nixie_clock_h
#define nixie_clock_h

#include <optional>

#include "esp_event.h"   //for wifi event
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "clock_iface.h"
#include "in14_nixie_tube.h"
#include "led_controller.h"
#include "sleep_info.h"
#include "time_info.h"
#include "web_server.h"
#include "wifi_info.h"

class NixieClock : public IClock {
  public:
    NixieClock();
    ~NixieClock() = default;
    void initialize();

    virtual std::optional<LedInfo> onGetLedInfo() const override;
    virtual void onSetLedInfo(const LedInfo& ledInfo) override;
    virtual std::optional<SleepInfo> onGetSleepInfo() const override;
    virtual void onSetSleepInfo(const SleepInfo& sleepInfo) override;
    virtual std::optional<WifiInfo> onGetWifiInfo() const override;
    virtual void onSetWifiInfo(const WifiInfo& wifiInfo) override;
    virtual std::optional<TimeInfo> onGetTimeInfo() const override;
    virtual void onSetTimeInfo(const TimeInfo& timeInfo) override;

  private:
    enum class WifiMode { Ap, Sta };

    WifiMode setupWifi(const WifiInfo& wifiInfo);
    bool initializeWifiInStationMode(const WifiInfo& wifiInfo);
    void initializeWifiInApMode(const WifiInfo& wifiInfo);
    void setupCaptivePortal();
    void startMdnsService(const WifiInfo& wifiInfo);
    void initializeSNTP();
    bool isInSleepMode();
    static void loopTask(void* param);
    bool startShowCurrentTimeTask(void);
    static void showCurrentTimeTask(void* param);
    void handleSleepMode();

    LedController mLedController;
    In14NixieTube mNixieTube;
    SleepInfo mSleepInfo;
    TimeInfo mTimeInfo;
    WebServer mWebServer;
    TaskHandle_t mShowCurrentTimeTaskHandle;
};
#endif   // nixie_clock_h