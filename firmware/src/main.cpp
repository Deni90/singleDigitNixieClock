#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <RtcDS3231.h>
#include <Ticker.h>
#include <WiFiClient.h>
#include <Wire.h>

#include "ClockFace.h"
#include "ConfigStore.h"
#include "In14NixieTube.h"
#include "LedController.h"
#include "NixieClockInterface.h"
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
ClockFace clockFace(ledController, nixieTube);
DNSServer dnsServer;
NixieClockInterface nci(ledController, rtc, clockFace);
WebServer webServer(WEBSERVER_PORT, nci);

volatile uint32_t counter = 0;
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
 * Function is incrementing a counter every time an interupt is generated.
 * Since DS3231's SQW pin is the source of the interupt, the counter will be
 * incremented every second.
 */
void IRAM_ATTR HandleInterrupt() { counter++; }

/* Set these to your desired credentials. */
const char* ssid = "ESPap";
const char* password = "thereisnospoon";

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

    Serial.printf("Reading config from EEPROM ... ");
    LedInfo li;
    ConfigStore::LoadLedConfiguration(li);
    Serial.println("Done");

    Serial.printf("Initializing RGB led ... ");
    ledController.Initialize(li);
    Serial.println("Done");

    Serial.printf("Initializing Nixie tube ... ");
    nixieTube.Initialize();
    Serial.println("Done");

    Serial.printf("Initializing real-time clock ... ");
    rtc.Begin();
    rtc.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_1Hz);
    rtc.SetSquareWavePin(DS3231SquareWavePin_ModeClock, false);
    Serial.println("Done");

    Serial.printf("Enabling interrupt for SQW ... ");
    pinMode(INTERRUPT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), HandleInterrupt,
                    FALLING);
    Serial.println("Done");

    Serial.printf("Updating led state ... ");
    RtcDateTime previousDateTime = rtc.GetDateTime();
    while (previousDateTime.Second() == rtc.GetDateTime().Second()) {
        delay(1);
    }
    timer.attach_ms(TIMER_PERIOD, HandleTimer);
    counter = rtc.GetDateTime().Second();
    Serial.println("Done");

    Serial.print("Mounting LittleFS...");
    if (!LittleFS.begin()) {
        Serial.println("Failed");
    } else {
        Serial.println("Done");
    }

    Serial.print("Configuring access point...");
    WiFi.softAP(ssid, password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.println("Done");

    Serial.print("Starting DNS server...");
    dnsServer.start(DNS_PORT, "*", myIP);
    Serial.println("Done");

    Serial.print("Initializing web server...");
    webServer.Initialize();
    Serial.println("Done");

    Serial.println();

    Serial.print("AP IP address: ");
    Serial.println(myIP);
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

    if (counter >= SECONDS_IN_MINUTE) {
        counter = 0;

        RtcDateTime now = rtc.GetDateTime();
        char str[20];   // declare a string as an array of chars
        sprintf(str, "%d/%d/%d %02d:%02d:%02d",   //%d allows to print an
                                                  // integer to the string
                now.Year(),                       // get year method
                now.Month(),                      // get month method
                now.Day(),                        // get day method
                now.Hour(),                       // get hour method
                now.Minute(),                     // get minute method
                now.Second()                      // get second method
        );
        Serial.printf("Time: %s\n", str);

        // show time on nixie tube.
        clockFace.ShowTime(now, CURRENT_TIME_REPEAT_NUM);
    }

    clockFace.Handle(globalClock);
}