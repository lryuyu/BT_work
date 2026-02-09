#include <BLEDevice.h>
#include "BluetoothManager.h"
#include "config.h"

BluetoothManager::BluetoothManager(): config() {}

class BluetoothManager::ConfigCharacteristicCallbacks final : public BLECharacteristicCallbacks {
  BluetoothManager* manager;
public:
  explicit ConfigCharacteristicCallbacks(BluetoothManager* mgr) : manager(mgr) {}
  void onWrite(BLECharacteristic *pCharacteristic) override {
    const std::string rxValue = pCharacteristic->getValue();
    if (!rxValue.empty()) {
      const auto received = String(rxValue.c_str());
      const int firstComma = received.indexOf(',');
      const int secondComma = received.indexOf(',', firstComma + 1);
      const int thirdComma = received.indexOf(',', secondComma + 1);

      if (firstComma > 0 && secondComma > firstComma && thirdComma > secondComma) {
        manager->config.ssid = received.substring(0, firstComma);
        manager->config.password = received.substring(firstComma + 1, secondComma);
        manager->config.ip = received.substring(secondComma + 1, thirdComma);
        manager->config.port = static_cast<uint16_t>(received.substring(thirdComma + 1).toInt());
        manager->config.isConfigReceived = true;

        Serial.printf("Received BLE Config:\nSSID:%s\nPASS:%s\nIP:%s\nPORT:%d\n",
                      manager->config.ssid.c_str(),
                      manager->config.password.c_str(),
                      manager->config.ip.c_str(),
                      manager->config.port);

        if (manager->pServer) {
          BLEAdvertising* adv = manager->pServer->getAdvertising();
          if (adv) {
            Serial.println("Restarting BLE advertising after config write.");
            adv->start();
          }
        }
      } else {
        Serial.println("Invalid config format received via BLE");
      }
    }
  }
};

class BluetoothManager::ConnectionServerCallbacks final : public BLEServerCallbacks {
  BluetoothManager* manager;
public:
  explicit ConnectionServerCallbacks(BluetoothManager* mgr) : manager(mgr) {}

  void onConnect(BLEServer* pServer) override {
    Serial.println("BLE client connected");
  }

  void onDisconnect(BLEServer* pServer) override {
    Serial.println("BLE client disconnected, restarting advertising");
    pServer->getAdvertising()->start();
  }
};

void BluetoothManager::setupBLE() {
  BLEDevice::init(BLE_CONFIG_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ConnectionServerCallbacks(this));

  BLEService *pService = pServer->createService(BLE_SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      BLE_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
                    );
  pCharacteristic->setCallbacks(new ConfigCharacteristicCallbacks(this));
  pCharacteristic->setValue("Ready");
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();

  BLEAdvertisementData advData;
  constexpr uint16_t appearance = BLE_ADV_APPEARANCE;
  advData.setAppearance(appearance);

  pAdvertising->setAdvertisementData(advData);
  pAdvertising->start();
  Serial.println("BLE Advertising started");
}

void BluetoothManager::begin() {
  setupBLE();
}

BLEConfig BluetoothManager::getConfig() {
  return config;
}

bool BluetoothManager::hasNewConfig() const {
  return config.isConfigReceived;
}

void BluetoothManager::clearNewConfigFlag() {
  config.isConfigReceived = false;
}






// #include <BLEDevice.h>
// #include <cstring>
// #include "BluetoothManager.h"
// #include "config.h"
//
//
// BluetoothManager::BluetoothManager() : deviceConnected(false) {}
//
// // BLE连接回调
// class BluetoothManager::ConnectionServerCallbacks final : public BLEServerCallbacks {
// private:
//     BluetoothManager& manager;
// public:
//     explicit ConnectionServerCallbacks(BluetoothManager& mgr) : manager(mgr) {}
//
//     void onConnect(BLEServer* pBLEServer) override {
//         manager.deviceConnected = true;
//         Serial.println("[BLE] Client connected");
//     }
//
//     void onDisconnect(BLEServer* pBLEServer) override {
//         manager.deviceConnected = false;
//         Serial.println("[BLE] Client disconnected, restart advertising");
//         pBLEServer->getAdvertising()->start();
//     }
// };
//
// // BLE数据接收回调
// class BLEConfigCallback : public BLECharacteristicCallbacks {
// public:
//     BLEConfig& config;
//     explicit BLEConfigCallback(BLEConfig& cfg) : config(cfg) {}
//
//
//     void onWrite(BLECharacteristic *const pChar) override {
//         if (pChar == nullptr) {
//             Serial.println("[BLE] Error: Null characteristic pointer");
//             return;
//         }
//
//         const std::string& data = pChar->getValue();
//
//         constexpr size_t maxDataLen = 128;
//         if (data.empty() || data.length() > maxDataLen) {
//             Serial.printf("[BLE] Invalid data length: %zu (max: %zu)\n", data.length(), maxDataLen);
//             return;
//         }
//
//         // 配置格式：ssid|password|ip|port
//         const String configStr(data.c_str());
//         const int idx1 = configStr.indexOf('|');
//         const int idx2 = configStr.indexOf('|', idx1 + 1);
//         const int idx3 = configStr.indexOf('|', idx2 + 1);
//
//
//         if (idx1 <= 0 || idx2 <= idx1 + 1 || idx3 <= idx2 + 1 || idx3 >= configStr.length() - 1) {
//             Serial.printf("[BLE] Invalid config format: %s (expected: ssid|password|ip|port)\n", configStr.c_str());
//             return;
//         }
//
//         config.ssid = configStr.substring(0, idx1);
//         config.password = configStr.substring(idx1 + 1, idx2);
//         config.ip = configStr.substring(idx2 + 1, idx3);
//
//         const String portStr = configStr.substring(idx3 + 1);
//         const long port = portStr.toInt();
//         if (port < 1 || port > 65535) {
//             Serial.printf("[BLE] Invalid port number: %s (must be 1-65535)\n", portStr.c_str());
//             return;
//         }
//         config.port = static_cast<uint16_t>(port);
//
//         config.isConfigReceived = true;
//         Serial.printf("[BLE] Config received successfully - SSID: %s, IP: %s, Port: %u\n",
//                       config.ssid.c_str(), config.ip.c_str(), config.port);
//     }
// };
//
// // BLE初始化
// void BluetoothManager::setupBLE() {
//     BLEDevice::init(BLE_CONFIG_NAME);
//
//     pServer = BLEDevice::createServer();
//     if (pServer == nullptr) {
//         Serial.println("[BLE] Failed to create BLE server");
//         return;
//     }
//     pServer->setCallbacks(new ConnectionServerCallbacks(*this));
//
//     BLEService *pService = pServer->createService(BLE_SERVICE_UUID);
//
//     if (pService == nullptr) {
//         Serial.println("[BLE] Failed to create BLE service");
//         return;
//     }
//
//     // 绑定配置解析回调（全局bleConfig）
//     pChar = pService->createCharacteristic(
//         BLE_CHARACTERISTIC_UUID,
//         BLECharacteristic::PROPERTY_READ |
//         BLECharacteristic::PROPERTY_WRITE |
//         BLECharacteristic::PROPERTY_NOTIFY
//         );
//
//     extern BLEConfig bleConfig;
//     pChar->setCallbacks(new BLEConfigCallback(bleConfig));
//
//     pService->start();
//     BLEAdvertising *pAdvertising = pServer->getAdvertising();
//     pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
//     pAdvertising->setAppearance(BLE_ADV_APPEARANCE);
//     pAdvertising->setScanResponse(true);
//     pAdvertising->setMinPreferred(0x00);
//     pAdvertising->setMaxPreferred(0x00);
//     //pAdvertising->start();
//     BLEDevice::startAdvertising();
//     Serial.println("[BLE] Advertising started");
// }
//
//
// void BluetoothManager::begin() {
//     setupBLE();
// }
//
//
// void BluetoothManager::sendData(const sensor_data_t *data) const {
//     if (!deviceConnected || data == nullptr) return;
//     uint8_t buf[sizeof(sensor_data_t)];
//     memcpy(buf, data, sizeof(buf));
//     pChar->setValue(buf, sizeof(buf));
//     pChar->notify();
// }
//
//
// bool BluetoothManager::isDeviceConnected() const {
//     return deviceConnected;
// }