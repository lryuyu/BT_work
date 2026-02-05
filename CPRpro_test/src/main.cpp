#include <Arduino.h>
#include "SensorFilter.h"
#include "BluetoothManager.h"
#include "SocketManager.h"
#include "WiFiManager.h"
#include "config.h"


WiFiManager wifiManager;
SocketClient socketClient;
sensor_data_t sensorData;


bool hasBLEConfig = false;
BLEConfig bleConfig;


unsigned long lastSendTime = 0;

void setup() {

  Serial.begin(115200);
  PN532_init();
  delay(100);

  sensor_init();
  Serial.println("Sensor init OK");


}



void loop() {

  if (hasBLEConfig) {

    wifiManager.begin(bleConfig.ssid, bleConfig.password);

    socketClient.reset();

    hasBLEConfig = false;
    Serial.println("[System] BLE config applied");
  }


  wifiManager.maintainConnection();


  if (WiFiManager::isConnected() && bleConfig.port != 0 && !socketClient.isConnected()) {
    socketClient.connectToServer(bleConfig.ip, bleConfig.port);
  }

  socketClient.maintainConnection();

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


    //BLE_send_data(&sensorData);
  }

  // const unsigned long now = millis();
  // static unsigned long lastDebugTime = 0;
  // if (now - lastDebugTime > 5000) {
  //   lastDebugTime = now;
  //   sensor_read_all_filtered(&sensorData);
  //   Serial.println("≤‚ ‘:");
  //   Serial.printf("lp_1=%d , lp_2=%d , lp_3=%d , lp_4=%d , lp_5=%d, AED_1=%d , AED_2=%d",
  //     sensorData.lp_1 , sensorData.lp_2 , sensorData.lp_3, sensorData.lp_4, sensorData.lp_5 , sensorData.AED_0 , sensorData.AED_1);
  // }


  delay(10);
}
