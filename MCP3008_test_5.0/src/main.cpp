#include <SPI.h>
#include <Arduino.h>

// 现在你已改用新的 CS 引脚
const int PIN_CS1 = 10;   // ADC1 (压力传感器大概率在这里) → D10
const int PIN_CS2 = 9;    // ADC2 (如果板上有第二个 MCP3008) → D9

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("\n=== Nano ESP32 + MCP3008 双CS 测试（CS1=D10, CS2=D9）===");
  Serial.println("接线确认： D13=SCK, D11=MOSI, D12=MISO");
  Serial.println("CS1 → D10 (ADC1),  CS2 → D9 (ADC2，如果有)");

  // 强制初始化所有相关引脚
  pinMode(PIN_CS1, OUTPUT);
  digitalWrite(PIN_CS1, HIGH);

  pinMode(PIN_CS2, OUTPUT);
  digitalWrite(PIN_CS2, HIGH);

  pinMode(13, OUTPUT);   // SCK
  pinMode(11, OUTPUT);   // MOSI
  pinMode(12, INPUT);    // MISO

  // 手动测试 CS1 (D10) 是否能翻转
  Serial.println("\n手动翻转 CS1 (D10) 测试...");
  for (int i = 0; i < 3; i++) {
    Serial.print("  第 "); Serial.print(i + 1); Serial.println(" 次拉低...");
    digitalWrite(PIN_CS1, LOW);
    delay(400);
    digitalWrite(PIN_CS1, HIGH);
    delay(400);
  }
  Serial.println("CS1 (D10) 翻转完成 → 请立即看示波器 D10 是否有 3 次高-低-高方波");

  // 手动测试 CS2 (D9) 是否能翻转
  Serial.println("\n手动翻转 CS2 (D9) 测试...");
  for (int i = 0; i < 3; i++) {
    Serial.print("  第 "); Serial.print(i + 1); Serial.println(" 次拉低...");
    digitalWrite(PIN_CS2, LOW);
    delay(400);
    digitalWrite(PIN_CS2, HIGH);
    delay(400);
  }
  Serial.println("CS2 (D9) 翻转完成 → 请看示波器 D9 是否有方波");

  SPI.begin();           // 默认使用 Nano ESP32 的 VSPI (D11/D12/D13)
  delay(200);

  Serial.println("\nSPI 初始化完成，开始读取通道...");
}

uint16_t readMCP3008(int csPin, uint8_t channel) {
  if (channel > 7) return 0;

  // 防止总线冲突，确保另一个 CS 是高电平
  int otherCS = (csPin == PIN_CS1) ? PIN_CS2 : PIN_CS1;
  digitalWrite(otherCS, HIGH);

  digitalWrite(csPin, LOW);
  delayMicroseconds(30);

  SPI.transfer(0x01);                        // Start bit
  uint8_t high = SPI.transfer((channel << 4) | 0b10000000);
  uint8_t low  = SPI.transfer(0x00);

  digitalWrite(csPin, HIGH);

  return ((high & 0x03) << 8) | low;
}

void loop() {
  Serial.println("\n===== ADC1 (CS=D10) 所有通道 =====");
  for (int ch = 0; ch < 8; ch++) {
    uint16_t raw = readMCP3008(PIN_CS1, ch);
    float volt = (raw * 3.3f) / 1023.0f;
    Serial.print("CH"); Serial.print(ch);
    Serial.print(": raw="); Serial.print(raw);
    Serial.print(" → "); Serial.print(volt, 3); Serial.println(" V");
    delay(40);
  }

  // 如果你确认有第二个 MCP3008，才取消下面注释
  /*
  Serial.println("\n===== ADC2 (CS=D9) 所有通道 =====");
  for (int ch = 0; ch < 8; ch++) {
    uint16_t raw = readMCP3008(PIN_CS2, ch);
    float volt = (raw * 3.3f) / 1023.0f;
    Serial.print("CH"); Serial.print(ch);
    Serial.print(": raw="); Serial.print(raw);
    Serial.print(" → "); Serial.print(volt, 3); Serial.println(" V");
    delay(40);
  }
  */

  Serial.println("-------------------------------");
  delay(1500);   // 方便你同时观察串口 + 示波器
}
