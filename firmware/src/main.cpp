#include <ArduinoJson.h>
#include <Base64.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <RtcDS3231.h>
#include <Ticker.h>
#include <WiFiClient.h>
#include <Wire.h>

#include "ConfigStore.h"
#include "In14NixieTube.h"
#include "LedController.h"
#include "NixieClock.h"
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

constexpr uint32_t TIMER_PERIOD = 1;        // ms
constexpr uint32_t UPDATE_LED_PERIOD = 4;   // ms 255* 4 = ~1s
constexpr uint32_t SECONDS_IN_MINUTE = 60;

constexpr int WEBSERVER_PORT = 80;
constexpr uint8_t DNS_PORT = 53;

constexpr uint8_t CURRENT_TIME_REPEAT_NUM = 3;
}   // namespace

Ticker timer;
RtcDS3231<TwoWire> rtc(Wire);
LedController ledController(LED_PIN);
In14NixieTube nixieTube(D0_PIN, D1_PIN, D2_PIN, D3_PIN);
NixieClock nixieClock(ledController, nixieTube, rtc);
DNSServer dnsServer;
WebServer webServer(WEBSERVER_PORT, nixieClock);
uint32_t globalClock = 0;
uint32_t ledControllerClock = 0;

/**
 * @brief Function that is called every millisecond
 */
void HandleTimer() {
    globalClock++;
    ledControllerClock++;
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
        // show time on nixie tube.
        Serial.printf("Current date & time: %d/%d/%d %02d:%02d:%02d\n",
                      now.Year(),     // get year method
                      now.Month(),    // get month method
                      now.Day(),      // get day method
                      now.Hour(),     // get hour method
                      now.Minute(),   // get minute method
                      now.Second()    // get second method
        );
        nixieClock.ShowTime(now, CURRENT_TIME_REPEAT_NUM);
    }
}

/**
 * @brief Initialize necessary modules
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

    Serial.print("Configuring Access Point...");
    WifiInfo wi;
    ConfigStore::LoadWifiInfo(wi);
    char decodedPassword[128];
    Base64.decode(decodedPassword, const_cast<char*>(wi.GetPassword().c_str()),
                  wi.GetPassword().length());

    WiFi.softAP(wi.GetSSID(), decodedPassword);
    IPAddress myIP = WiFi.softAPIP();
    Serial.println("Done");

    Serial.print("Starting DNS server...");
    dnsServer.start(DNS_PORT, "*", myIP);
    Serial.println("Done");

    Serial.print("Initializing Web server...");
    webServer.Initialize();
    Serial.println("Done");

    Serial.println();
    Serial.println("---------------------------------------------------");
    Serial.println("                 | Configuration |                 ");
    Serial.println("---------------------------------------------------");
    String buffer;
    serializeJsonPretty(li.ToJson(), buffer);
    Serial.printf("LedInfo:\n%s\n\n", buffer.c_str());
    serializeJsonPretty(si.ToJson(), buffer);
    Serial.printf("SleepInfo:\n%s\n\n", buffer.c_str());
    serializeJsonPretty(wi.ToJson(), buffer);
    Serial.printf("WifiInfo:\n%s\n\n", buffer.c_str());
    Serial.printf("Wifi AP IP address: %s\n", myIP.toString().c_str());
    Serial.printf("Wifi decoded password: %s\n", decodedPassword);
    Serial.println("---------------------------------------------------");
    Serial.println();
}

/**
 * @brief Do things in loop
 */
void loop() {
    dnsServer.processNextRequest();

    if (ledControllerClock >= UPDATE_LED_PERIOD) {
        ledControllerClock = 0;
        ledController.Update();
    }

    nixieClock.Handle(globalClock);
}