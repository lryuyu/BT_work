#include <Arduino.h>
#include "SensorFilter.h"
#include "BluetoothManager.h"
#include "SocketManager.h"
#include "WiFiManager.h"
#include "config.h"

BLEConfig bleConfig;
WiFiManager wifiManager;
SocketClient socketClient;
BluetoothManager bleManager;
sensor_data_t sensorData;

bool hasBLEConfig = false;

unsigned long lastSendTime = 0;

// ////////////////////////////////////////////////////////////
// const unsigned long DEBUG_LOG_INTERVAL = 1000;
// unsigned long lastDebugPrintTime = 0;
// ////////////////////////////////////////////////////////////////

void setup() {

  Serial.begin(115200);
  delay(100);
  Serial.println("System initialized");

  sensor_init();

  PN532_init();

  bleManager.begin();
  Serial.println("Sensor init OK");
}

void loop() {

  if (bleConfig.isConfigReceived) {
    hasBLEConfig = true;
    wifiManager.begin(bleConfig.ssid, bleConfig.password);

    socketClient.reset();
    bleConfig.isConfigReceived = false;
    Serial.println("[System] BLE config applied");
  }


  wifiManager.maintainConnection();

  if (WiFiManager::isConnected() && bleConfig.port != 0 && !socketClient.isConnected()) {
    const BLEConfig cfg = bleManager.getConfig();
    socketClient.connectToServer(bleConfig.ip, bleConfig.port);
  }


  const unsigned long now = millis();

  // //////////////////////////////////////////////////////////////////////
  // // 新增：固定间隔打印传感器原始数据（无论是否发送到服务器）
  // if (now - lastDebugPrintTime >= DEBUG_LOG_INTERVAL) {
  //   lastDebugPrintTime = now;
  //   // 读取传感器数据并打印
  //   sensor_read_all_filtered(&sensorData);
  //   Serial.println("\n==================== Sensor Data Log ====================");
  //   Serial.printf("lp_1: %d | lp_2: %d | lp_3: %d | lp_4: %d | lp_5: %d\n",
  //                 sensorData.lp_1, sensorData.lp_2, sensorData.lp_3, sensorData.lp_4, sensorData.lp_5);
  //   Serial.printf("pp_0: %d | pp_1: %d\n", sensorData.pp_0, sensorData.pp_1);
  //   Serial.printf("nose_0: %d | nose_1: %d\n", sensorData.nose_0, sensorData.nose_1);
  //   Serial.printf("head_0: %d | chin_0: %d\n", sensorData.head_0, sensorData.chin_0);
  //   Serial.printf("AF: %d | DIS: %d | AED_0: %d | AED_1: %d\n",
  //                 sensorData.AF, sensorData.DIS, sensorData.AED_0, sensorData.AED_1);
  //   Serial.println("=========================================================\n");
  // }///////////////////////////////////////////////////////////////////////

  if (socketClient.isConnected() && (now - lastSendTime >= SEND_INTERVAL)) {

    lastSendTime = now;

    sensor_read_all_filtered(&sensorData);

    String jsonData;
    // jsonData += "{\"timestamp\":" + String(now) + ",";
    jsonData += "" + String(sensorData.lp_1) + "-";
    jsonData += "" + String(sensorData.lp_2) + "-";
    jsonData += "" + String(sensorData.lp_3) + "-";
    jsonData += "" + String(sensorData.lp_4) + "-";
    jsonData += "" + String(sensorData.lp_5) + "-";
    //jsonData += "" + String(sensorData.lp_6) + "-";
    //jsonData += "" + String(sensorData.lp_7) + "-";
    //jsonData += "" + String(sensorData.lp_8) + "-";
    //jsonData += "" + String(sensorData.lp_9) + "-";
    jsonData += "" + String(sensorData.pp_0) + "-";
    jsonData += "" + String(sensorData.pp_1) + "-";
    jsonData += "" + String(sensorData.nose_0) + "-";
    jsonData += "" + String(sensorData.nose_1) + "-";
    jsonData += "" + String(sensorData.head_0) + "-";
    jsonData += "" + String(sensorData.chin_0) + "";
    jsonData += "" + String(sensorData.AF) + "";
    jsonData += "" + String(sensorData.DIS) + "";
    jsonData += "" + String(sensorData.AED_0) + "";
    jsonData += "" + String(sensorData.AED_1) + "";


    if (socketClient.sendData(jsonData)) {
      Serial.printf("[System] Send data: %s\n", jsonData.c_str());
    } else {
      Serial.println("[System] Send data failed");
    }


    //BLE_send_data(&sensorData);
  }

  // const unsigned long now = millis();
  // static unsigned long lastDebugTime = 0;
  // if (now - lastDebugTime > 5000) {
  //   lastDebugTime = now;
  //   sensor_read_all_filtered(&sensorData);
  //   Serial.println("????:");
  //   Serial.printf("lp_1=%d , lp_2=%d , lp_3=%d , lp_4=%d , lp_5=%d, AED_1=%d , AED_2=%d",
  //     sensorData.lp_1 , sensorData.lp_2 , sensorData.lp_3, sensorData.lp_4, sensorData.lp_5 , sensorData.AED_0 , sensorData.AED_1);
  // }


  delay(1000);
}
