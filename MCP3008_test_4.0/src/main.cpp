#include <SPI.h>
#include <Arduino.h>

// const int SCK_PIN = 13;
// void setup() { pinMode(SCK_PIN, OUTPUT); }
// void loop() { digitalWrite(SCK_PIN, !digitalRead(SCK_PIN)); delay(500); }

// const int SCK_PIN = 13;
// const int PIN_MOSI = 11;
// const int PIN_MISO = 12;
// const int PIN_CS   = 3;
// void setup() { SPI.begin(SCK_PIN, PIN_MISO, PIN_MOSI, PIN_CS);
//     pinMode(PIN_CS, OUTPUT);
//     digitalWrite(PIN_CS, HIGH);  }
// void loop() {
//     digitalWrite(PIN_CS, LOW);
//     SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
//     uint8_t v = SPI.transfer(0xAA);  // 随便发一个字节
//     SPI.endTransaction();
//     digitalWrite(PIN_CS, HIGH);
//     delay(100);
//}

//（1）SCK MOSI CS都跳
// const int PIN_SCK  = 13;
// const int PIN_MOSI = 11;
// const int PIN_CS   = 3;
//
// void setup() {
//     pinMode(PIN_SCK,  OUTPUT);
//     pinMode(PIN_MOSI, OUTPUT);
//     pinMode(PIN_CS,   OUTPUT);
//     digitalWrite(PIN_CS, HIGH);   // 空闲
// }

// //（2）
// const int PIN_HEART = 2;
//
// void setup() {
//     pinMode(PIN_HEART, OUTPUT);
//     digitalWrite(PIN_HEART, LOW);
//     SPI.begin();           // 默认 SPI 引脚
// }
//
// void loop() {
//     digitalWrite(PIN_HEART, HIGH);
//     // 高频率传输
//     for (int i = 0; i < 100; i++) {
//         SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
//         SPI.transfer(0xAA);
//         SPI.endTransaction();
//     }
//     delay(10);
// }

//（3）软SPI

// ---------- 引脚定义 ----------
const int PIN_SCK  = 13;   // MCP3008 CLK
const int PIN_MOSI = 11;   // MCP3008 DIN
const int PIN_MISO = 12;   // MCP3008 DOUT
const int PIN_CS   = 3;    // MCP3008 CS

// ---------- 基本时序延时 ----------
// 可以先用 2us，若不稳定可增大到 5~10us
inline void spiDelay() {
  delayMicroseconds(2);
}

// ---------- 发送 1 bit，MODE0：CPOL=0, CPHA=0 ----------
inline void spiWriteBit(bool bitVal) {
  digitalWrite(PIN_MOSI, bitVal ? HIGH : LOW);
  spiDelay();
  digitalWrite(PIN_SCK, HIGH);
  spiDelay();
  digitalWrite(PIN_SCK, LOW);
  spiDelay();
}

// ---------- 读 1 bit（在 SCK 上升沿后读取 MISO） ----------
inline bool spiReadBit() {
  digitalWrite(PIN_SCK, HIGH);
  spiDelay();
  bool bitVal = digitalRead(PIN_MISO);
  digitalWrite(PIN_SCK, LOW);
  spiDelay();
  return bitVal;
}

// ---------- 读 MCP3008 指定通道（0~7），返回 0~1023 ----------
int mcp3008ReadChannel(uint8_t ch) {
  if (ch > 7) return 0;

  digitalWrite(PIN_CS, LOW);   // 选中 MCP3008
  spiDelay();

  // 发送起始位 1
  spiWriteBit(1);

  // 发送 single/diff 位：1 = single-ended
  spiWriteBit(1);

  // 发送通道选择位：CH2, CH1, CH0（从高到低）
  spiWriteBit((ch & 0x04) != 0);  // CH2
  spiWriteBit((ch & 0x02) != 0);  // CH1
  spiWriteBit((ch & 0x01) != 0);  // CH0

  // 再读一个 null bit（丢弃）
  spiReadBit();

  // 接着读取 10 位结果（从 MSB 开始）
  int result = 0;
  for (int i = 9; i >= 0; i--) {
    bool bitVal = spiReadBit();
    if (bitVal) {
      result |= (1 << i);
    }
  }

  digitalWrite(PIN_CS, HIGH);  // 结束转换
  spiDelay();

  return result;  // 0~1023，对应 0~Vref
}

void setup() {
  pinMode(PIN_SCK,  OUTPUT);
  pinMode(PIN_MOSI, OUTPUT);
  pinMode(PIN_MISO, INPUT);
  pinMode(PIN_CS,   OUTPUT);

  digitalWrite(PIN_SCK, LOW);   // 空闲低
  digitalWrite(PIN_MOSI, LOW);
  digitalWrite(PIN_CS,  HIGH);  // 空闲高

  Serial.begin(115200);         // 串口看数据
}

void loop() {
  // 依次读取 CH0~CH7
  for (uint8_t ch = 0; ch < 8; ch++) {
    int value = mcp3008ReadChannel(ch);
    Serial.print("CH");
    Serial.print(ch);
    Serial.print(": ");
    Serial.print(value);
    Serial.print("  ");
  }
  Serial.println();

  delay(200); // 200ms 更新一次
}






//
// void loop() {
//     // 模拟“假 SPI”：CS 拉低后，SCK 和 MOSI 以 1Hz 跳
//     digitalWrite(PIN_CS, LOW);
//     digitalWrite(PIN_SCK, !digitalRead(PIN_SCK));
//     digitalWrite(PIN_MOSI, !digitalRead(PIN_MOSI));
//     delay(500);
//     digitalWrite(PIN_CS, HIGH);
//     delay(500);
// }




// #define CS_PIN 3
//
// // 指定使用 ESP32 的 FSPI 总线
// SPIClass SPI_1(FSPI);
//
// int readMCP3008(byte channel) {
//     byte command = 0b11000000 | (channel << 3);
//     digitalWrite(CS_PIN, LOW);
//
//     SPI_1.transfer(command);
//     byte high = SPI_1.transfer(0x00);
//     byte low  = SPI_1.transfer(0x00);
//
//     digitalWrite(CS_PIN, HIGH);
//
//     return ((high & 0x01) << 8) | low;
// }
//
// void setup() {
//     Serial.begin(115200);
//     pinMode(CS_PIN, OUTPUT);
//     digitalWrite(CS_PIN, HIGH);
//
//     // 重点：这里必须对应你的接线
//     SPI.begin(13, 12, 11, -1);
//     //SPI_1.begin(48, 47, 38, CS_PIN);
//     // SCK=GPIO48 → D13
//     // MISO=GPIO47 → D12
//     // MOSI=GPIO38 → D11
//
//     Serial.println("开始读取 MCP3008...");
// }
//
// void loop() {
//     int adc = readMCP3008(0);
//     float v = adc * (3.3 / 1023.0);
//     Serial.printf("CH0: %d   %.2fV\n", adc, v);
//     delay(500);
// }