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
        // 用成员变量，而非局部变量
        if (millis() - _connectStartTime > 2000) {
            Serial.println("WiFi connection timeout, retry later");
            _connecting = false;
        }
        return;  // 避免重复进入重连逻辑
    }

    const unsigned long now = millis();
    if (now - lastReconnectAttempt >= reconnectInterval) {
        Serial.println("WiFi disconnected, reconnecting...");
        WiFi.disconnect();
        WiFi.begin(currentSSID.c_str(), currentPass.c_str());
        _connecting = true;
        _connectStartTime = millis();  // 初始化连接开始时间
        lastReconnectAttempt = now;
    }
}

void WiFiManager::reset() {
    WiFi.disconnect(true);
    currentSSID = "";
    currentPass = "";
    lastReconnectAttempt = 0;
}