#include <ArduinoJson.h>
#include <Base64.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>
#include <RtcDS3231.h>
#include <Ticker.h>
#include <WiFiClient.h>
#include <Wire.h>

#include "ConfigStore.h"
#include "In14NixieTube.h"
#include "LedController.h"
#include "NixieClock.h"
#include "TimeManager.h"
#include "WebServer.h"

namespace {
constexpr uint8_t LED_PIN = D3;   // RGB led
// BCD2DEC encoder = Nixie tube
constexpr uint8_t D0_PIN = D4;
constexpr uint8_t D1_PIN = D8;
constexpr uint8_t D2_PIN = D5;
constexpr uint8_t D3_PIN = D6;

constexpr uint8_t INTERRUPT_PIN = D7;   // DS3231's SQW pin

constexpr unsigned long UART_BAUDRATE = 115200;

constexpr uint32_t TIMER_PERIOD = 1;             // ms
constexpr uint32_t UPDATE_LED_PERIOD = 4;        // ms 255* 4 = ~1s
constexpr uint32_t TIME_SYNC_PERIOD = 3600000;   // 1Hour in milliseconds
constexpr uint32_t SECONDS_IN_MINUTE = 60;
constexpr uint8_t MINUTES_IN_HOUR = 60;

constexpr int WEBSERVER_PORT = 80;
constexpr uint8_t DNS_PORT = 53;
constexpr const char* DEFAULT_AP_SSID = "NixieClock";

constexpr uint8_t CURRENT_TIME_REPEAT_NUM = 3;

Ticker timer;
RtcDS3231<TwoWire> rtc(Wire);
LedController ledController(LED_PIN);
In14NixieTube nixieTube(D0_PIN, D1_PIN, D2_PIN, D3_PIN);
TimeManager timeManager;
NixieClock nixieClock(ledController, nixieTube, rtc, timeManager);
DNSServer dnsServer;
WebServer webServer(WEBSERVER_PORT, nixieClock);
uint32_t globalClock = 0;
uint32_t ledControllerClock = 0;
uint32_t timeSyncClock = 0;
bool apMode = false;
}   // namespace

/**
 * @brief Function that is called every millisecond
 */
void HandleTimer() {
    globalClock++;
    ledControllerClock++;
    timeSyncClock++;
}

/**
 * @brief Interrupt handler for the INTERRUPT_PIN connected to RTC
 *
 * Show time on the nixie tube every minute
 */
void IRAM_ATTR HandleInterrupt() {
    RtcDateTime now = rtc.GetDateTime();
    if (now.Year() == c_OriginYear) {
        return;
    }
    if (now.Second() == 0) {
        nixieClock.ShowTime(now, CURRENT_TIME_REPEAT_NUM);
    }
}

/**
 * @brief Initialize Wifi in Station Mode
 *
 * @return true if successfully connected to a network
 */
bool InitializeWifiInStationMode(const WifiInfo& wifiInfo) {
    char decodedPassword[128];
    Base64.decode(decodedPassword,
                  const_cast<char*>(wifiInfo.GetPassword().c_str()),
                  wifiInfo.GetPassword().length());
    Serial.printf("Connecting to %s", wifiInfo.GetSSID().c_str());
    WiFi.setHostname(wifiInfo.GetHostname().c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiInfo.GetSSID(), decodedPassword);
    uint8_t counter = 100;
    IPAddress myIP;
    while (WiFi.status() != WL_CONNECTED) {
        printf(".");
        delay(200);
        if (counter-- == 0) {
            break;
        }
    }
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(" Failed");
        WiFi.disconnect();
        return false;
    }
    Serial.println(" Done");
    return true;
}

/**
 * @brief Initialize Wifi in AP mode
 */
void InitializeWifiInApMode() {
    Serial.printf("Setting up soft AP ...");
    WiFi.softAP(DEFAULT_AP_SSID);
    apMode = true;
    Serial.println("Done");
}

void SynchroniseTime() {
    Serial.printf("Synchronising time... ");
    RtcDateTime timeNow;
    if (timeManager.GetDateTime(timeNow)) {
        rtc.SetDateTime(timeNow);
        Serial.println("Done");
    } else {
        Serial.println("Failed");
    }
}

/**
 * @brief Initialize all the necessary modules
 */
void setup() {
    Serial.begin(UART_BAUDRATE);

    Serial.println("");
    Serial.println("   _  _ _____  _____ ___    ___ _    ___   ___ _  __");
    Serial.println(" | \\| |_ _\\ \\/ /_ _| __|  / __| |  / _ \\ / __| |/ /");
    Serial.println(" | .` || | >  < | || _|  | (__| |_| (_) | (__| \' < ");
    Serial.println(
        " |_|\\_|___/_/\\_\\___|___|  \\___|____\\___/ \\___|_|\\_\\");
    Serial.println("");

    Serial.print("Mounting LittleFS... ");
    if (!LittleFS.begin()) {
        Serial.println("Failed");
    } else {
        Serial.println("Done");
    }

    Serial.printf("Initializing RGB led... ");
    LedInfo li;
    ConfigStore::LoadLedInfo(li);
    ledController.Initialize(li);
    Serial.println("Done");

    Serial.printf("Initializing Nixie tube... ");
    nixieTube.Initialize();
    Serial.println("Done");

    Serial.printf("Initializing Real-Time Clock... ");
    rtc.Begin();
    rtc.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_1Hz);
    rtc.SetSquareWavePin(DS3231SquareWavePin_ModeClock, false);
    Serial.println("Done");

    Serial.printf("Enabling interrupt for SQW... ");
    pinMode(INTERRUPT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), HandleInterrupt,
                    FALLING);
    Serial.println("Done");

    WifiInfo wi;
    ConfigStore::LoadWifiInfo(wi);
    if (!InitializeWifiInStationMode(wi)) {
        InitializeWifiInApMode();
    }

    TimeInfo ti;
    ConfigStore::LoadTimeInfo(ti);
    timeManager.Initialize(ti.GetOffset());
    SynchroniseTime();

    Serial.printf("Initializing timer(s)... ");
    RtcDateTime previousDateTime = rtc.GetDateTime();
    while (previousDateTime.Second() == rtc.GetDateTime().Second()) {
        delay(1);
    }
    timer.attach_ms(TIMER_PERIOD, HandleTimer);
    Serial.println("Done");

    Serial.printf("Initializing NixieClock... ");
    SleepInfo si;
    ConfigStore::LoadSleepInfo(si);
    nixieClock.Initialize(si);
    nixieClock.ShowTime(rtc.GetDateTime(), 1);
    Serial.println("Done");

    if (apMode) {
        Serial.printf("Initializing captive portal... ");
        dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    }

    Serial.printf("Initializing mDNS... ");
    MDNS.begin(wi.GetHostname());
    Serial.println("Done");

    Serial.print("Initializing Web server... ");
    webServer.Initialize();
    Serial.println("Done");
}

/**
 * @brief Do things in loop
 */
void loop() {
    if (apMode) {
        dnsServer.processNextRequest();
    }
    MDNS.update();

    if (ledControllerClock >= UPDATE_LED_PERIOD) {
        ledControllerClock = 0;
        ledController.Update();
    }

    if (timeSyncClock >= TIME_SYNC_PERIOD) {
        timeSyncClock = 0;
        SynchroniseTime();
    }

    nixieClock.Handle(globalClock);
}