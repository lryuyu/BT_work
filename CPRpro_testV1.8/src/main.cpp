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

String currentServerIP;
uint16_t currentServerPort = 0;
unsigned long lastNFCRead = 0;
unsigned long lastSendTime = 0;
unsigned long lastDebugPrintTime = 0;

const unsigned long DEBUG_LOG_INTERVAL = 1000;

const char* getNfcStatusString(uint32_t status) {
  if (status == 1) return "UID Match(1)";
  if (status == 0) return "UID Mismatch(0)";
  return "No Card";
}

// 辅助函数：在控制台格式化打印传感器数据
void printSensorDataToConsole() {
  Serial.println("\n========== SENSOR & NFC DATA ==========");
  Serial.printf("LP Sensors   | 1:%-4u 2:%-4u 3:%-4u 4:%-4u 5:%-4u\n",
                sensorData.lp_1, sensorData.lp_2, sensorData.lp_3, sensorData.lp_4, sensorData.lp_5);
  Serial.printf("PP Sensors   | 0:%-4u 1:%-4u\n",
                sensorData.pp_L, sensorData.pp_R);
  Serial.printf("Face Sensors | Nose0:%-4u Nose1:%-4u Head0:%-4u Chin0:%-4u\n",
                sensorData.nose_L, sensorData.nose_R, sensorData.head_0, sensorData.chin_0);
  Serial.printf("Misc Sensors | AF:%-4u DIS:%-4u\n",
                sensorData.AF, sensorData.DIS);
  Serial.printf("NFC 1 (AED_0)| %s\n", getNfcStatusString(sensorData.AED_L));
  Serial.printf("NFC 2 (AED_1)| %s\n", getNfcStatusString(sensorData.AED_R));
  Serial.println("=======================================\n");
}

// void printSensorDataToConsole() {
//
//   Serial.printf("DIS:%d\n",sensorData.DIS);
//
// }

void setup() {
  delay(5000);
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println("System initialized");

  MCP3008_init();

  // 测试读取第一个MCP3008的通道0，重复10次
  for (int i = 0; i < 10; i++) {
    uint16_t val = MCP3008_read(ADC_1, 0);  // 读取通道0
    Serial.printf("MCP3008#0 CH0 = %d\n", val);
    delay(500);
  }

  sensor_init();
  PN532_init();

  bleManager.begin();
  Serial.println("Sensor init OK");
}

void loop() {

  if (bleManager.hasNewConfig()) {
    BLEConfig cfg = bleManager.getConfig();
    wifiManager.begin(cfg.ssid, cfg.password);
    currentServerIP = cfg.ip;
    currentServerPort = cfg.port;
    socketClient.reset();
    bleManager.clearNewConfigFlag();
    Serial.println("[System] BLE config applied");
  }


  wifiManager.maintainConnection();

  if (WiFiManager::isConnected() && currentServerIP != 0 && !socketClient.isConnected()) {
    socketClient.connectToServer(currentServerIP, currentServerPort);
  }


  const unsigned long now = millis();

  if (now - lastNFCRead > 500)
  {
    lastNFCRead = now;
    sensorData.AED_L = PN532_1_read_uid();
    sensorData.AED_R = PN532_2_read_uid();
  }

  sensor_read_all_filtered(&sensorData);

  //本地控制台打印
  if (now - lastDebugPrintTime >= DEBUG_LOG_INTERVAL) {
    lastDebugPrintTime = now;
    printSensorDataToConsole();
  }

  //网络匹配
  if (bleManager.hasNewConfig()) {
    BLEConfig cfg = bleManager.getConfig();
    wifiManager.begin(cfg.ssid, cfg.password);
    currentServerIP = cfg.ip;
    currentServerPort = cfg.port;
    socketClient.reset();
    bleManager.clearNewConfigFlag();
    Serial.println("[System] BLE config applied. Conneting to WIFI...");
  }
  wifiManager.maintainConnection();
  if (WiFiManager::isConnected() && currentServerPort != 0 && !socketClient.isConnected()) {
    socketClient.connectToServer(currentServerIP, currentServerPort);
  }
  socketClient.maintainConnection();

  if (socketClient.isConnected() && (now - lastSendTime >= SEND_INTERVAL)) {

    lastSendTime = now;

    sensor_read_all_filtered(&sensorData);

    String data = String(sensorData.lp_1) + "," +
                      String(sensorData.lp_2) + "," +
                      String(sensorData.lp_3) + "," +
                      String(sensorData.lp_4) + "," +
                      String(sensorData.lp_5) + "," +
                      String(sensorData.pp_L) + "," +
                      String(sensorData.pp_R) + "," +
                      String(sensorData.nose_L) + "," +
                      String(sensorData.nose_R) + "," +
                      String(sensorData.head_0) + "," +
                      String(sensorData.chin_0) + "," +
                      String(sensorData.AF) + "," +
                      String(sensorData.DIS) + "," +
                      String(sensorData.AED_L) + "," +
                      String(sensorData.AED_R);

    data += "\n";

    if (socketClient.sendData(data)) {
      Serial.printf("[System] Send data: %s\n", data.c_str());
    } else {
      Serial.println("[System] Send data failed");
    }

  }

  socketClient.maintainConnection();


  delay(10);
}
