#include "SocketManager.h"
#include <WiFiClient.h>


bool SocketClient::connectToServer(const String& ip, const uint16_t port) {

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
    // 未连接 + 已配置IP/端口 → 尝试重连
    if (!isConnected() && serverIP.length() > 0 && serverPort != 0) {
        const unsigned long now = millis();
        // 达到重连间隔
        if (now - lastReconnectAttempt >= reconnectInterval) {
            Serial.println("[Socket] Reconnecting...");
            // 尝试重连
            const bool res = connectToServer(serverIP, serverPort);
            lastReconnectAttempt = now;
            if (!res) {
                Serial.println("[Socket] Reconnect failed");
            }
        }
    }
}

/**
 * @brief 重置Socket配置
 */
void SocketClient::reset() {
    disconnect();          // 断开连接
    serverIP = "";         // 清空IP
    serverPort = 0;        // 清空端口
    lastReconnectAttempt = 0; // 重置重连时间
    connected = false;     // 重置连接标记
}