#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <Arduino.h>
#include <BLEServer.h>
#include <BLEDevice.h>
#include <BLECharacteristic.h>
#include "SensorFilter.h"

// BLE配置结构体：存储WiFi/服务器配置信息
struct BLEConfig {
    String ssid;          // WiFi名称
    String password;      // WiFi密码
    String ip;            // 服务器IP
    uint16_t port = 0;    // 服务器端口
    bool isConfigReceived = false; // 是否收到新配置
};

class BluetoothManager {
public:
    BluetoothManager();
    void begin();
    void sendData(const sensor_data_t *data) const;
    bool isDeviceConnected() const;

private:
    BLEServer* pServer = nullptr;
    BLECharacteristic* pChar = nullptr;
    bool deviceConnected = false;

    void setupBLE();
    class ConnectionServerCallbacks;
};

#endif