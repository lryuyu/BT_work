#include "SocketManager.h"
#include  "esp_task_wdt.h"
#include <WiFiClient.h>


bool SocketClient::connectToServer(const String& ip, const uint16_t port) {

    client.connect(ip.c_str(),port,2000);
    esp_task_wdt_reset();  // Î¹¹·

    if (client.connected()) {
        client.stop();
    }
    Serial.printf("[Socket] Connecting to %s:%d...\n", ip.c_str(), port);



    const bool result = client.connect(ip.c_str(), port);
    connected = result;


    if (result) {
        Serial.println("[Socket] Connected");
        serverIP = ip;
        serverPort = port;
    } else {
        Serial.println("[Socket] Connection failed");
    }
    return result;
}


void SocketClient::disconnect() {
    if (client.connected()) {
        client.stop();
        Serial.println("[Socket] Disconnected");
    }
    connected = false;
}


bool SocketClient::isConnected() {

    if (connected && !client.connected()) {
        connected = false;
    }
    return connected;
}


bool SocketClient::sendData(const String& data) {

    if (!isConnected()) return false;


    const size_t bytesSent = client.print(data);

    client.flush();

    return bytesSent == data.length();
}


void SocketClient::maintainConnection() {

    if (!isConnected() && serverIP.length() > 0 && serverPort != 0) {
        const unsigned long now = millis();

        if (now - lastReconnectAttempt >= reconnectInterval) {
            Serial.println("[Socket] Reconnecting...");

            const bool res = connectToServer(serverIP, serverPort);
            lastReconnectAttempt = now;
            if (!res) {
                Serial.println("[Socket] Reconnect failed");
            }
        }
    }
}


void SocketClient::reset() {
    disconnect();
    serverIP = "";
    serverPort = 0;
    lastReconnectAttempt = 0;
    connected = false;
}