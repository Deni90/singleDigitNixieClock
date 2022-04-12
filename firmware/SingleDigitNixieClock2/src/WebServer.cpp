#include <LittleFS.h>
#include <ArduinoJson.h>

#include "WebServer.h"

namespace
{
    constexpr int HTTP_200_OK = 200;
    constexpr int HTTP_400_BAD_REQUEST = 400;
    constexpr int HTTP_500_INTERNAL_SERVER_ERROR = 500;
}

WebServer& WebServer::Instance()
{
    static WebServer ws;
    return ws;
}

void WebServer::Initialize(int port, ClockInterface* callback)
{
    this->callback = callback;

    webServer = new ESP8266WebServer(port);
    webServer->onNotFound(std::bind(&WebServer::HandleWebRequests, this));
    webServer->on("/", HTTP_GET, std::bind(&WebServer::HandleRoot, this));
    webServer->on("/backlight", HTTP_GET, std::bind(&WebServer::HandleBacklight, this));
    webServer->on("/backlight/state", HTTP_POST, std::bind(&WebServer::HandleSetBacklightState, this));
    webServer->on("/backlight/color", HTTP_POST, std::bind(&WebServer::HandleSetBacklightColor, this));
    webServer->begin();
}

void WebServer::Handle()
{
    if(webServer)
        webServer->handleClient();
}

WebServer::WebServer()
:webServer(nullptr)
,callback(nullptr)
{
}

WebServer::~WebServer()
{
    if(webServer)
    {
        delete webServer;
    }
}

bool WebServer::LoadFromLittleFS(String path)
{
    if(!webServer)
    {
        return false;
    }

    bool returnValue = true;
    Serial.println("Load path: " + path);
    String dataType = "text/plain";

    if(path.endsWith("/")) path += "index.html";

    if(path.endsWith(".src"))
        path = path.substring(0, path.lastIndexOf("."));
    else if(path.endsWith(".html"))
        dataType = "text/html";
    else if(path.endsWith(".htm"))
        dataType = "text/html";
    else if(path.endsWith(".css"))
        dataType = "text/css";
    else if(path.endsWith(".js"))
        dataType = "application/javascript";
    else if(path.endsWith(".png"))
        dataType = "image/png";
    else if(path.endsWith(".gif"))
        dataType = "image/gif";
    else if(path.endsWith(".jpg"))
        dataType = "image/jpeg";
    else if(path.endsWith(".ico"))
        dataType = "image/x-icon";
    else if(path.endsWith(".xml"))
        dataType = "text/xml";
    else if(path.endsWith(".pdf"))
        dataType = "application/pdf";
    else if(path.endsWith(".zip"))
        dataType = "application/zip";
    if (LittleFS.exists(path))
    {
        File dataFile = LittleFS.open(path.c_str(), "r");
        if(webServer->hasArg("download")) dataType = "application/octet-stream";

        if(webServer->streamFile(dataFile, dataType) != dataFile.size())
        {
            // Serial.println("Error: streamed file has different size!");
            // returnValue = false;
        }
        dataFile.close();
    }
    else
    {
        Serial.println("Error: Path does not exist and will be redirect to root:");
        Serial.println(path);
        returnValue = LoadFromLittleFS("/");
    }
    return returnValue;
}

void WebServer::HandleRoot()
{
    HandleWebRequests();
}

void WebServer::HandleWebRequests()
{
    if(!webServer)
    {
        return;
    }
    if(!LoadFromLittleFS(webServer->uri()))
    {
        Serial.println("Error: handleWebRequests");
        String message = "File Not Detected\n\n";
        message += "URI: ";
        message += webServer->uri();
        message += "\nMethod: ";
        message += (webServer->method() == HTTP_GET) ? "GET" : "POST";
        message += "\nArguments: ";
        message += webServer->args();
        message += "\n";
        for(uint8_t i = 0; i < webServer->args(); i++)
        {
            message += " NAME:" + webServer->argName(i) +
            "\n VALUE:" + webServer->arg(i) + "\n";
        }
        webServer->send(404, "text/plain", message);
        Serial.println(message);
    }
}

void WebServer::HandleBacklight()
{
    if(!webServer || !callback)
    {
        Serial.println("Error: Web server not initalized");
        webServer->send(HTTP_500_INTERNAL_SERVER_ERROR, "");
        return;
    }

    LedInfo li = callback->OnGetBacklightData();

    StaticJsonDocument<200> doc;
    char messageBuffer[200];

    doc["state"] = static_cast<uint8_t>(li.GetState());
    doc["R"] = li.GetR();
    doc["G"] = li.GetG();
    doc["B"] = li.GetB();
    doc["A"] = li.GetA();

    serializeJsonPretty(doc, messageBuffer);

    webServer->send(HTTP_200_OK, "application/json", messageBuffer);

    Serial.printf("HandleBacklight: %s\n", messageBuffer);
}

void WebServer::HandleSetBacklightState()
{
    Serial.println("HandleSetBacklightState: " + webServer->arg("plain"));

    if(!webServer || !callback)
    {
        Serial.println("Error: Web server not initalized");
        webServer->send(HTTP_500_INTERNAL_SERVER_ERROR, "");
        return;
    }

    if(webServer->hasArg("state"))
    {
        uint8_t state = webServer->arg("state").toInt();
        if(callback->OnSetBacklightState(state))
        {
            webServer->send(HTTP_200_OK, "");
        }
        else
        {
            webServer->send(HTTP_400_BAD_REQUEST, "");
        }
    } else {
        Serial.println("Error handleSetLed: missing argument state!");
        webServer->send(HTTP_400_BAD_REQUEST, "");
    }
}

void WebServer::HandleSetBacklightColor()
{
    Serial.println("HandleSetBacklightState: " + webServer->arg("plain"));

        if(!webServer || !callback)
    {
        Serial.println("Error: Web server not initalized");
        webServer->send(HTTP_500_INTERNAL_SERVER_ERROR, "");
        return;
    }

    if(webServer->hasArg("R") && webServer->hasArg("G")
        && webServer->hasArg("B") && webServer->hasArg("A"))
    {
        uint8_t r, g, b, a;
        r = webServer->arg("R").toInt();
        g = webServer->arg("G").toInt();
        b = webServer->arg("B").toInt();
        a = webServer->arg("A").toInt();
        callback->OnSetBacklightColor(r, g, b, a);
        webServer->send(HTTP_200_OK, "");
    }
    else
    {
        Serial.println("Error HandleSetBacklightColor: missing argument(s)!");
        webServer->send(HTTP_400_BAD_REQUEST, "");
    }
}