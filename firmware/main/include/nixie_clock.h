#ifndef nixie_clock_h
#define nixie_clock_h

#include <optional>

#include "esp_event.h"   //for wifi event
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "clock_iface.h"
#include "ds3231.h"
#include "i2c_bus.h"
#include "in14_nixie_tube.h"
#include "led_controller.h"
#include "sleep_info.h"
#include "time_info.h"
#include "web_server.h"
#include "wifi_info.h"
#include "wifi_manager.h"

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
    void setupCaptivePortal();
    void startMdnsService(const WifiInfo& wifiInfo);
    void initializeSNTP();
    static void timeSyncNotificationCallback(struct timeval* tv);
    bool isInSleepMode();
    static void loopTask(void* param);
    bool startShowCurrentTimeTask(void);
    static void showCurrentTimeTask(void* param);
    void handleSleepMode();
    time_t timegmRtc(struct tm* tm);

    LedController mLedController;
    In14NixieTube mNixieTube;
    WifiManager mWifiManager;
    WebServer mWebServer;
    SleepInfo mSleepInfo;
    TimeInfo mTimeInfo;
    TaskHandle_t mShowCurrentTimeTaskHandle;
    I2cBus mI2c;
    Ds3231 mRtc;
    bool mLastSleepModeStatus;
};
#endif   // nixie_clock_h