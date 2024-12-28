#include "WifiInfo.h"

WifiInfo::WifiInfo(const String& ssid, const String& password)
    : ssid(ssid), password(password) {}

String WifiInfo::GetSSID() const { return ssid; }

void WifiInfo::SetSSID(const String& value) { ssid = value; }

String WifiInfo::GetPassword() const { return password; }

void WifiInfo::SetPassword(const String& value) { password = value; }

JsonDocument WifiInfo::ToJson() const {
    JsonDocument doc;
    doc["SSID"] = ssid;
    doc["password"] = password;
    return doc;
}