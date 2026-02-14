#include <Arduino.h>
#include "SensorFilter.h"
#include "BluetoothManager.h"
#include "SocketManager.h"
#include "WiFiManager.h"
#include "config.h"

WiFiManager wifiManager;
SocketClient socketClient;
BluetoothManager bleManager;
sensor_data_t sensorData;

bool hasBLEConfig = false;

unsigned long lastSendTime = 0;

void setup() {

  Serial.begin(115200);
  delay(100);
  Serial.println("????????§µ?");

  // sensor_init();
  //
  // PN532_init();

  bleManager.begin();
  Serial.println("Sensor init OK");
}

void loop() {

  if (bleManager.hasNewConfig()) {
    hasBLEConfig = true;
    BLEConfig cfg = bleManager.getConfig();
    wifiManager.begin(cfg.ssid, cfg.password);

    socketClient.reset();

    bleManager.clearNewConfigFlag();
    Serial.println("[System] BLE config applied");
  }


  wifiManager.maintainConnection();

  if (WiFiManager::isConnected() && hasBLEConfig && !socketClient.isConnected()) {
    BLEConfig cfg = bleManager.getConfig();
    socketClient.connectToServer(cfg.ip, cfg.port);
  }


  const unsigned long now = millis();

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


  }



  delay(10);
}
