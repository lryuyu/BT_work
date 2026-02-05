#ifndef SOCKET_MANAGER_H
#define SOCKET_MANAGER_H

#include <Arduino.h>
#include <WiFiClient.h>
#include "config.h"

class SocketClient {
public:
    SocketClient() = default;
    ~SocketClient() {disconnect();} // 析构时断开连接

    /**
     * @brief 连接到TCP服务器
     * @param ip 服务器IP地址
     * @param port 服务器端口
     * @return 连接结果（true=成功）
     */
    bool connectToServer(const String& ip, uint16_t port);

    /**
     * @brief 断开TCP连接
     */
    void disconnect();

    /**
     * @brief 检查TCP连接状态
     * @return 连接状态（true=已连接）
     */
    bool isConnected();

    /**
     * @brief 发送数据到TCP服务器
     * @param data 要发送的字符串
     * @return 发送结果（true=全部发送成功）
     */
    bool sendData(const String& data);

    /**
     * @brief 维护TCP连接（断线重连）
     */
    void maintainConnection();

    /**
     * @brief 重置Socket配置（断开+清空IP/端口）
     */
    void reset();

private:
    WiFiClient client;               // WiFi客户端对象
    String serverIP = "";            // 服务器IP
    uint16_t serverPort = 0;         // 服务器端口
    unsigned long lastReconnectAttempt = 0; // 上次重连时间
    const unsigned long reconnectInterval = SOCKET_RECONNECT_INTERVAL;
    bool connected = false;          // 连接状态标记
};

#endif