#include "WiFiManager.h"

void WiFiManager::begin(const String& ssid, const String& password) {
    currentSSID = ssid;
    currentPass = password;
    Serial.printf("[WiFi] Connecting to WiFi: %s\n", ssid.c_str());
    Serial.printf("[WiFi] Password: %s\n", password.c_str());
    WiFi.disconnect();
    WiFi.begin(ssid.c_str(), password.c_str());
    lastReconnectAttempt = millis();
}

bool WiFiManager::isConnected() {
    return WiFiClass::status() == WL_CONNECTED;
}

void WiFiManager::maintainConnection() {
    if (WiFiClass::status() != WL_CONNECTED) {
        const unsigned long now = millis();
        if (now - lastReconnectAttempt >= reconnectInterval) {
            Serial.println("[WiFi] WiFi disconnected, reconnecting...");
            Serial.printf("[WiFi] Current SSID: %s\n", currentSSID.c_str());
            Serial.printf("[WiFi] Current password: %s\n", currentPass.c_str());
            Serial.printf("[WiFi] WiFi status: %d\n", WiFiClass::status());
            
            if (currentSSID.isEmpty() || currentPass.isEmpty()) {
                Serial.println("[WiFi] ERROR: SSID or password is empty!");
            } else {
                WiFi.disconnect();
                WiFi.begin(currentSSID.c_str(), currentPass.c_str());
            }
            lastReconnectAttempt = now;
        }
    } else {
        static bool connected = false;
        if (!connected) {
            Serial.println("[WiFi] WiFi connected successfully!");
            Serial.printf("[WiFi] IP address: %s\n", WiFi.localIP().toString().c_str());
            connected = true;
        }
    }
}

void WiFiManager::reset() {
    WiFi.disconnect(true);
    currentSSID = "";
    currentPass = "";
    lastReconnectAttempt = 0;
}