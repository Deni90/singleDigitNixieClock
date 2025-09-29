/******************************************************************************
 * File:    wifi_info.cpp
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Implements WifiInfo data class
 ******************************************************************************/

#include "wifi_info.h"

WifiInfo::WifiInfo(const std::string& hostname, const std::string& ssid,
                   const WifiAuthType& authType, const std::string& password)
    : mHostname(hostname), mSsid(ssid), mAuthType(authType),
      mPassword(password) {}

std::string WifiInfo::getHostname() const { return mHostname; }

void WifiInfo::setHostname(const std::string& value) { mHostname = value; }

std::string WifiInfo::getSSID() const { return mSsid; }

void WifiInfo::setSSID(const std::string& value) { mSsid = value; }

WifiAuthType WifiInfo::getAuthType() const { return mAuthType; }

void WifiInfo::setAuthType(const WifiAuthType& value) { mAuthType = value; }

std::string WifiInfo::getPassword() const { return mPassword; }

void WifiInfo::setPassword(const std::string& value) { mPassword = value; }