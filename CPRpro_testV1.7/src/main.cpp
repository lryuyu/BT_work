#include <Arduino.h>
#include "SensorFilter.h"
#include "BluetoothManager.h"
#include "SocketManager.h"
#include "WiFiManager.h"
#include "config.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"pool.ntp.org",0,60000);
WiFiManager wifiManager;
SocketClient socketClient;
BluetoothManager bleManager;
sensor_data_t sensorData;
bool timeSynced = false;

String currentServerIP;
uint16_t currentServerPort = 0;
unsigned long lastNtpAttempt = 0;
unsigned long lastNFCRead = 0;
unsigned long lastSendTime = 0;
unsigned long lastDebugPrintTime = 0;
const unsigned long NTP_RETRY_INTERVAL = 30000;
const unsigned long DEBUG_LOG_INTERVAL = 1000;


void printSensorDataToConsole() {
  unsigned long now = millis();
    String data = "";
        data += String(sensorData.lp_2) + "," ;
        data += String(sensorData.lp_3) + "," ;
        data += String(sensorData.lp_4) + "," ;
        data += String(sensorData.lp_5) + "," ;
        data += String(sensorData.pp_L) + "," ;
        data += String(sensorData.AF) + "," ;
        data += String(sensorData.DIS) + "," ;
        data += String(sensorData.AED_L) + "," ;
        data += String(sensorData.AED_R) + "," ;
        data += String(now) + "\r\n" ;
        //example: xx,xx,xx,xx,xx,pp,af,dis,1,2,timestamp\r\n
    Serial.println(data);
}



void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("System starting...");

  delay(5000);
  MCP3008_init();
  sensor_init();
  PN532_init();

  bleManager.begin();
  Serial.println("Sensor init OK");
  timeClient.begin();
}

void loop() {

  const unsigned long now = millis();

  if (bleManager.hasNewConfig()) {
    BLEConfig cfg = bleManager.getConfig();
    wifiManager.begin(cfg.ssid, cfg.password);
    currentServerIP = cfg.ip;
    currentServerPort = cfg.port;
    socketClient.reset();
    bleManager.clearNewConfigFlag();
    Serial.println("[System] BLE config applied");
    timeSynced = false;
    lastNtpAttempt = 0;
  }


  wifiManager.maintainConnection();

  if (WiFiManager::isConnected() && !timeSynced && (now - lastNtpAttempt >= NTP_RETRY_INTERVAL)) {
    lastNtpAttempt = now;
    if (timeClient.update()) {
      timeSynced = true;
      Serial.println("[System] NTP time synchronized");
    }else {
      Serial.println("[System] NTP update failed");
    }
  }

  if (WiFiManager::isConnected() && currentServerPort != 0 && !socketClient.isConnected()) {
    socketClient.connectToServer(currentServerIP, currentServerPort);
  }
  socketClient.maintainConnection();

  //循环读取传感器的数据
  sensor_read_all_filtered(&sensorData);

  if (now - lastNFCRead > 500)
  {
    lastNFCRead = now;
    sensorData.AED_L = PN532_1_read_uid();
    sensorData.AED_R = PN532_2_read_uid();
  }


  if (now - lastDebugPrintTime >= DEBUG_LOG_INTERVAL) {
    lastDebugPrintTime = now;
    printSensorDataToConsole();
  }

  if (socketClient.isConnected() && (now - lastSendTime >= SEND_INTERVAL)) {

    lastSendTime = now;


  unsigned long long absTimestamp;
    if (timeSynced) {
      absTimestamp = timeClient.getEpochTime() * 1000ULL + (millis() % 1000);
    }else {
      absTimestamp = now;
    }

    String data = String(sensorData.lp_1) + "," +
                      String(sensorData.lp_2) + "," +
                      String(sensorData.lp_3) + "," +
                      String(sensorData.lp_4) + "," +
                      String(sensorData.lp_5) + "," +
                      String(sensorData.pp_L) + "," +
                      //String(sensorData.pp_R) + "," +
                      //String(sensorData.nose_L) + "," +
                      //String(sensorData.nose_R) + "," +
                      //String(sensorData.head_0) + "," +
                      //String(sensorData.chin_0) + "," +
                      String(sensorData.AF) + "," +
                      String(sensorData.DIS) + "," +
                      String(sensorData.AED_L) + "," +
                      String(sensorData.AED_R) +","+
                        String(absTimestamp)+"\r\n";


    if (socketClient.sendData(data)) {
      Serial.printf("[System] Send data: %s\n", data.c_str());
    } else {
      Serial.println("[System] Send data failed");
    }

  }

  socketClient.maintainConnection();


  delay(10);
}
