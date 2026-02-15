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

String currentServerIP;
uint16_t currentServerPort = 0;
unsigned long lastNFCRead = 0;
unsigned long lastSendTime = 0;



// ////////////////////////////////////////////////////////////
// const unsigned long DEBUG_LOG_INTERVAL = 1000;
// unsigned long lastDebugPrintTime = 0;
// ////////////////////////////////////////////////////////////////

void setup() {

  Serial.begin(115200);
  delay(100);
  Serial.println("System initialized");

  // sensor_init();
  //
  // PN532_init();

  bleManager.begin();
  Serial.println("Sensor init OK");
}

void loop() {

  if (bleConfig.isConfigReceived) {
    BLEConfig cfg = bleManager.getConfig();
    wifiManager.begin(cfg.ssid, cfg.password);
    currentServerIP = cfg.ip;
    currentServerPort = cfg.port;
    socketClient.reset();
    bleManager.clearNewConfigFlag();
    Serial.println("[System] BLE config applied");
  }


  wifiManager.maintainConnection();

  if (WiFiManager::isConnected() && bleConfig.port != 0 && !socketClient.isConnected()) {
    socketClient.connectToServer(currentServerIP, currentServerPort);
  }


  const unsigned long now = millis();

  // if (millis() - lastNFCRead > 500)
  // {
  //   lastNFCRead = millis();
  //   AED_0 = PN532_1_read_uid();
  //   AED_1 = PN532_2_read_uid();
  // }


  if (socketClient.isConnected() && (now - lastSendTime >= SEND_INTERVAL)) {

    lastSendTime = now;

    sensor_read_all_filtered(&sensorData);

    String data = String(sensorData.lp_1) + "," +
                      String(sensorData.lp_2) + "," +
                      String(sensorData.lp_3) + "," +
                      String(sensorData.lp_4) + "," +
                      String(sensorData.lp_5) + "," +
                      String(sensorData.pp_0) + "," +
                      String(sensorData.pp_1) + "," +
                      String(sensorData.nose_0) + "," +
                      String(sensorData.nose_1) + "," +
                      String(sensorData.head_0) + "," +
                      String(sensorData.chin_0) + "," +
                      String(sensorData.AF) + "," +
                      String(sensorData.DIS) + "," +
                      String(sensorData.AED_0) + "," +
                      String(sensorData.AED_1);


    if (socketClient.sendData(data)) {
      Serial.printf("[System] Send data: %s\n", data.c_str());
    } else {
      Serial.println("[System] Send data failed");
    }

  }
  if (millis() - lastNFCRead > 500)
  {
    lastNFCRead = millis();
    sensorData.AED_0 = PN532_1_read_uid();
    sensorData.AED_1 = PN532_2_read_uid();
  }
  socketClient.maintainConnection();


  delay(10);
}
