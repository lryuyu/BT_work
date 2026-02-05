#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H
#include <Arduino.h>
#include <WiFi.h>

class WiFiManager {
public:
    WiFiManager() = default;
    void begin(const String& ssid, const String& password);
    static bool isConnected();
    void maintainConnection();
    void reset();

private:
    String currentSSID = "";
    String currentPass = "";
    unsigned long lastReconnectAttempt = 0;
    const unsigned long reconnectInterval = 5000;
};

#endif