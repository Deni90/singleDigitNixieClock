#include "WifiInfo.h"

WifiInfo::WifiInfo(const String& hostname, const String& ssid,
                   const String& password)
    : hostname(hostname), ssid(ssid), password(password) {}

String WifiInfo::GetHostname() const { return hostname; }

void WifiInfo::SetHostname(const String& value) { hostname = value; }

String WifiInfo::GetSSID() const { return ssid; }

void WifiInfo::SetSSID(const String& value) { ssid = value; }

String WifiInfo::GetPassword() const { return password; }

void WifiInfo::SetPassword(const String& value) { password = value; }

JsonDocument WifiInfo::ToJson() const {
    JsonDocument doc;
    doc["hostname"] = hostname;
    doc["SSID"] = ssid;
    doc["password"] = password;
    return doc;
}