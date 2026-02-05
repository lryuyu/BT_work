#include <BLEDevice.h>
#include <cstring>
#include "BluetoothManager.h"
#include "config.h"


BluetoothManager::BluetoothManager() : deviceConnected(false) {}


class BluetoothManager::ConnectionServerCallbacks final : public BLEServerCallbacks {
private:
    BluetoothManager& manager;
public:
    explicit ConnectionServerCallbacks(BluetoothManager& mgr) : manager(mgr) {}

    void onConnect(BLEServer* pBLEServer) override {
        manager.deviceConnected = true;
        Serial.println("[BLE] Client connected");
    }

    void onDisconnect(BLEServer* pBLEServer) override {
        manager.deviceConnected = false;
        Serial.println("[BLE] Client disconnected, restart advertising");
        pBLEServer->getAdvertising()->start();
    }
};

// BLE初始化
void BluetoothManager::setupBLE() {
    BLEDevice::init(BLE_CONFIG_NAME);


    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ConnectionServerCallbacks(*this));

    BLEService *pService = pServer->createService(BLE_SERVICE_UUID);


    pChar = pService->createCharacteristic(
        BLE_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );


    pService->start();


    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->setAppearance(BLE_ADV_APPEARANCE);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMaxPreferred(0x12);
    pAdvertising->start();

    Serial.println("[BLE] Advertising started");
}

// 外部调用的初始化入口
void BluetoothManager::begin() {
    setupBLE();
}

// 发送传感器数据（核心推送逻辑）
void BluetoothManager::sendData(const sensor_data_t *data) const {
    if (!deviceConnected || data == nullptr) return;

    // 拷贝传感器数据到缓冲区并推送
    uint8_t buf[sizeof(sensor_data_t)];
    memcpy(buf, data, sizeof(buf));
    pChar->setValue(buf, sizeof(buf));
    pChar->notify();
}

// 获取设备连接状态
bool BluetoothManager::isDeviceConnected() const {
    return deviceConnected;
}