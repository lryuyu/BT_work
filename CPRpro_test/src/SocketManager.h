#ifndef SOCKET_MANAGER_H
#define SOCKET_MANAGER_H

#include <Arduino.h>
#include <WiFiClient.h>
#include "config.h"

class SocketClient {
public:
    SocketClient() = default;
    ~SocketClient() {disconnect();}

    bool connectToServer(const String& ip, uint16_t port);


    void disconnect();


    bool isConnected();


    bool sendData(const String& data);


    void maintainConnection();


    void reset();

private:
    WiFiClient client;
    String serverIP = "";
    uint16_t serverPort = 0;
    unsigned long lastReconnectAttempt = 0;
    const unsigned long reconnectInterval = SOCKET_RECONNECT_INTERVAL;
    bool connected = false;
};

#endif