#include "WiFiManager.h"

void WiFiManager::begin(const String& ssid, const String& password) {
    currentSSID = ssid;
    currentPass = password;
    Serial.printf("Connecting to WiFi: %s\n", ssid.c_str());
    WiFi.disconnect();
    WiFi.begin(ssid.c_str(), password.c_str());
    lastReconnectAttempt = millis();
}

bool WiFiManager::isConnected() {
    return WiFiClass::status() == WL_CONNECTED;
}

void WiFiManager::maintainConnection() {
    if (isConnected()) {
        _connecting = false;
        return;
    }

    if (_connecting) {
        unsigned long _connectStartTime = 0;
        if (millis() - _connectStartTime > 2000) {
            Serial.println("WiFi connection timeout, retry later");
            _connecting = false;
        }
    }

    const unsigned long now = millis();
    if (now - lastReconnectAttempt >= reconnectInterval) {
        unsigned long _connectStartTime = 0;
        Serial.println("WiFi disconnected, reconnecting...");
        WiFi.disconnect();
        WiFi.begin(currentSSID.c_str(), currentPass.c_str());
        _connecting = true;
        _connectStartTime = 0;
        lastReconnectAttempt = now;
    }
}

void WiFiManager::reset() {
    WiFi.disconnect(true);
    currentSSID = "";
    currentPass = "";
    lastReconnectAttempt = 0;
}