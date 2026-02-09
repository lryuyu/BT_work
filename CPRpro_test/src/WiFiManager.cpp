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
    if (isConnected()) return;

    const unsigned long now = millis();
    if (now - lastReconnectAttempt >= reconnectInterval) {
        Serial.println("WiFi disconnected, reconnecting...");
        WiFi.disconnect();
        WiFi.begin(currentSSID.c_str(), currentPass.c_str());
        lastReconnectAttempt = now;
    }
}

void WiFiManager::reset() {
    WiFi.disconnect(true);
    currentSSID = "";
    currentPass = "";
    lastReconnectAttempt = 0;
}