#include <Ticker.h>
#include <Wire.h>
#include <RtcDS3231.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <time.h>

#include "LedController.h"
#include "BCD2DecimalDecoder.h"

#define UART_BAUDRATE   115200
#define EEPROM_SIZE     512

#define SOFT_AP_SSID    "NixieClock"
#define SOFT_AP_PASS    "12345678"

#define HTTP_REST_PORT  80

#define LED_PIN         D3
#define LED_COUNT       1

#define INTERRUPT_PIN   D7

#define D0_PIN          D4
#define D1_PIN          D8
#define D2_PIN          D5
#define D3_PIN          D6

#define TIMER_PERIOD        1 //ms
#define UPDATE_LED_PERIOD   4 //ms 255* 4 = ~1s

#define SECONDS_IN_MINUTE   60

Ticker timer;
RtcDS3231<TwoWire> rtc(Wire);
Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
LedController ledController(led);
BCD2DecimalDecoder bcd2decimalDecoder( D0_PIN, D1_PIN, D2_PIN, D3_PIN );
ESP8266WebServer http_rest_server(HTTP_REST_PORT);

uint32_t ledControllerClock = 0;

volatile uint32_t counter= 0;

void GetLedInfo() {
    Serial.println("HTTP GET /led -->");
    StaticJsonDocument<200> doc;
    char messageBuffer[200];

    doc["R"] = ledController.GetR();
    doc["G"] = ledController.GetG();
    doc["B"] = ledController.GetB();
    doc["A"] = ledController.GetA();
    doc["state"] = ledController.GetState();
    serializeJsonPretty(doc, messageBuffer);
    Serial.printf("BODY:\n%s\n", messageBuffer);
    Serial.printf("RESP: %d\n", 200);
    http_rest_server.send(200, "application/json", messageBuffer);
    Serial.println("<--\n");
}

void ChangeLedConfig()
{
    Serial.println("HTTP POST /led -->");
    StaticJsonDocument<500> doc;
    String post_body = http_rest_server.arg("plain");
    Serial.printf("BODY:\n%s\n", http_rest_server.arg("plain").c_str());
    DeserializationError error = deserializeJson(doc, http_rest_server.arg("plain"));
    if(error)
    {
        Serial.printf("RESP: %d\n", 400);
        Serial.println("<--\n");
        http_rest_server.send(400);
        return;
    }

    uint8_t state = doc["state"];
    if( state > LedState::BREATHE )
    {
        Serial.printf("RESP: %d\n", 409);
        Serial.println("<--\n");
        http_rest_server.send(409);
        return;
    }

    ledController.SetColor(doc["R"], doc["G"], doc["B"], doc["A"]);
    ledController.SetState(static_cast<LedState>(state));

    EEPROM.begin(512);  //Initialize EEPROM
    uint16_t eepromAddr = 0;
    EEPROM.write(eepromAddr, doc["R"]);
    eepromAddr++;
    EEPROM.write(eepromAddr, doc["G"]);
    eepromAddr++;
    EEPROM.write(eepromAddr, doc["B"]);
    eepromAddr++;
    EEPROM.write(eepromAddr, doc["A"]);
    eepromAddr++;
    EEPROM.write(eepromAddr, state);
    eepromAddr++;
    EEPROM.commit();    //Store data to EEPROM

    Serial.printf("RESP: %d\n", 200);
    http_rest_server.send(200);
    Serial.println("<--\n");
}

void GetTime()
{
    Serial.println("HTTP GET /time -->");
    RtcDateTime now = rtc.GetDateTime();

    char str[20];   //declare a string as an array of chars
    sprintf(str, "%d/%d/%d %d:%d:%d",     //%d allows to print an integer to the string
            now.Year(),   //get year method
            now.Month(),  //get month method
            now.Day(),    //get day method
            now.Hour(),   //get hour method
            now.Minute(), //get minute method
            now.Second()  //get second method
            );
    StaticJsonDocument<200> doc;
    char messageBuffer[200];

    doc["time"] = str;
    serializeJsonPretty(doc, messageBuffer);
    Serial.printf("BODY:\n%s\n", messageBuffer);
    Serial.printf("RESP: %d\n", 200);
    http_rest_server.send(200, "application/json", messageBuffer);
    Serial.println("<--\n");
}

void SetTime()
{
    Serial.println("HTTP POST /time -->");
    StaticJsonDocument<500> doc;
    String post_body = http_rest_server.arg("plain");
    Serial.printf("BODY:\n%s\n", http_rest_server.arg("plain").c_str());
    DeserializationError error = deserializeJson(doc, http_rest_server.arg("plain"));
    if(error)
    {
        Serial.printf("RESP: %d\n", 400);
        Serial.println("<--\n");
        http_rest_server.send(400);
        return;
    }

    if(!doc["time"])
    {
        Serial.printf("RESP: %d\n", 409);
        Serial.println("<--\n");
        http_rest_server.send(409);
        return;
    }

    tm time;
    memset(&time, 0, sizeof(time));
    //YY/MM/DD HH:MM:SS
    if(!strptime(doc["time"], "%Y/%m/%d %H:%M:%S", &time))
    {
        Serial.println("Failed to parse time.");
        Serial.printf("RESP: %d\n", 409);
        Serial.println("<--\n");
        http_rest_server.send(409);
        return;
    }

    if(time.tm_year < 100)
    {
        Serial.printf("RESP: %d\n", 409);
        Serial.println("<--\n");
        http_rest_server.send(409);
        return;
    }

    char str[20];   //declare a string as an array of chars
    sprintf(str, "%d/%d/%d %d:%d:%d",     //%d allows to print an integer to the string
                             1900 + time.tm_year,   //get year method
                             1 + time.tm_mon,  //get month method
                             time.tm_mday,    //get day method
                             time.tm_hour,   //get hour method
                             time.tm_min, //get minute method
                             time.tm_sec  //get second method
                             );
    Serial.printf("Setting time to: %s\n", str);

    RtcDateTime newTime = RtcDateTime(time.tm_year - 100,
                                      time.tm_mon + 1,
                                      time.tm_mday,
                                      time.tm_hour,
                                      time.tm_min,
                                      time.tm_sec);
    rtc.SetDateTime(newTime);

    counter = time.tm_sec;

    ledController.Reset();

    Serial.printf("RESP: %d\n", 200);
    Serial.println("<--\n");
}

void HandleTimer()
{
    ledControllerClock++;
}

void ICACHE_RAM_ATTR HandleInterrupt() {
  counter++;
}

void ConfigRestServerRouting() {
    http_rest_server.on("/", HTTP_GET, []() {
        http_rest_server.send(200, "text/html",
            "Welcome to the ESP8266 REST Web Server");
    });
    http_rest_server.on("/led", HTTP_GET, GetLedInfo);
    http_rest_server.on("/led", HTTP_POST, ChangeLedConfig);
    http_rest_server.on("/time", HTTP_GET, GetTime);
    http_rest_server.on("/time", HTTP_POST, SetTime);
}

void setup() {
    Serial.begin(UART_BAUDRATE);

    Serial.printf("\n\nSINGLE TUBE NIXIE CLOCK\n\n");

    Serial.printf("Reading config from EEPROM ... ");
    EEPROM.begin(EEPROM_SIZE);  //Initialize EEPROM
    uint16_t eepromAddr = 0;
    uint8_t r = EEPROM.read(eepromAddr);
    eepromAddr++;
    uint8_t g = EEPROM.read(eepromAddr);
    eepromAddr++;
    uint8_t b = EEPROM.read(eepromAddr);
    eepromAddr++;
    uint8_t a = EEPROM.read(eepromAddr);
    eepromAddr++;
    uint8_t state = EEPROM.read(eepromAddr);
    eepromAddr++;
    EEPROM.commit();    //Store data to EEPROM
    Serial.println("Done");

    Serial.println("LED config:");
    Serial.printf("    R: %d\n", r);
    Serial.printf("    G: %d\n", g);
    Serial.printf("    B: %d\n", b);
    Serial.printf("    A: %d\n", a);
    Serial.printf("    state: %d\n", state);

    Serial.printf("Initializing RGB led ... ");
    ledController.Initialize(r, g, b, a, static_cast<LedState>(state));
    Serial.println("Done");

    Serial.printf("Initializing BCD to decimal decoder ... ");
    bcd2decimalDecoder.Initialize();
    Serial.println("Done");

    Serial.printf("Initializing real-time clock ... ");
    rtc.Begin();
    rtc.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_1Hz);
    rtc.SetSquareWavePin(DS3231SquareWavePin_ModeClock, false);
    Serial.println("Done");

    Serial.printf("Enabling interrupt for SQW ... ");
    pinMode(INTERRUPT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), HandleInterrupt, FALLING);
    Serial.println("Done");

    Serial.printf("Updating led state ... ");
    RtcDateTime previousDateTime = rtc.GetDateTime();
    while(previousDateTime.Second() == rtc.GetDateTime().Second())
    {
        delay(1);
    }
    timer.attach_ms(TIMER_PERIOD, HandleTimer);
    counter = rtc.GetDateTime().Second();
    Serial.println("Done");

    Serial.print("Setting soft-AP ... ");
    boolean result = WiFi.softAP(SOFT_AP_SSID, SOFT_AP_PASS);
    if(result == true)
    {
        Serial.println("Done");
    }
    else
    {
        Serial.println("Failed!");
    }

    Serial.print("Setting HTTP REST server ... ");
    ConfigRestServerRouting();
    http_rest_server.begin();
    Serial.println("Done");
}

void loop() {
    http_rest_server.handleClient();

    if(ledControllerClock >= UPDATE_LED_PERIOD)
    {
        ledControllerClock = 0;
        ledController.Update();
    }

    if(counter >= SECONDS_IN_MINUTE)
    {
        counter = 0;

        RtcDateTime now = rtc.GetDateTime();
        char str[20];   //declare a string as an array of chars
        sprintf(str, "%d/%d/%d %02d:%02d:%02d",     //%d allows to print an integer to the string
                now.Year(),   //get year method
                now.Month(),  //get month method
                now.Day(),    //get day method
                now.Hour(),   //get hour method
                now.Minute(), //get minute method
                now.Second()  //get second method
                );
        Serial.printf("Time: %s\n", str);
    }
}
