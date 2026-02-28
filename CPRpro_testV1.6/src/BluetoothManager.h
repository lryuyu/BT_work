#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <Arduino.h>
#include <BLEServer.h>

struct BLEConfig {
    String ssid;
    String password;
    String ip;
    uint16_t port;
    bool isConfigReceived = false;
};

class BluetoothManager {
public:
    BluetoothManager();
    void begin();
    BLEConfig getConfig();
    bool hasNewConfig() const;
    void clearNewConfigFlag();

private:
    BLEServer* pServer = nullptr;
    BLECharacteristic* pCharacteristic = nullptr;
    BLEConfig config;
    void setupBLE();
    class ConfigCharacteristicCallbacks;
    class ConnectionServerCallbacks;
};

#endif


// #ifndef BLUETOOTH_MANAGER_H
// #define BLUETOOTH_MANAGER_H
//
// #include <Arduino.h>
// #include <BLEServer.h>
// #include <BLEDevice.h>
// #include <BLECharacteristic.h>
// #include "SensorFilter.h"
//
//
// struct BLEConfig {
//     String ssid;
//     String password;
//     String ip;
//     uint16_t port = 0;
//     bool isConfigReceived = false;
// };
//
// class BluetoothManager {
// public:
//     BluetoothManager();
//     void begin();
//     void sendData(const sensor_data_t *data) const;
//     bool isDeviceConnected() const;
//    // bool parseConfig(const String& configStr, BLEConfig& config);
//
//
// private:
//     BLEServer* pServer = nullptr;
//     BLECharacteristic* pChar = nullptr;
//     bool deviceConnected = false;
//
//     void setupBLE();
//     class ConnectionServerCallbacks;
// };
//
// #endif