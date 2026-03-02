#include <Arduino.h>
#include <SPI.h>

// 定义片选引脚
#define CS_PIN 5

void setup() {
    // 启动串口调试
    Serial.begin(115200);

    // 设置 CS 引脚输出并默认释放（高电平）
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);

    // 初始化 SPI 总线（自动绑定 VSPI 默认引脚）
    SPI.begin();

    delay(1000);
    Serial.println("【ESP32 SPI 测试】开始读取 MCP3008 数据");
}

// 读取指定通道（0~7）的 ADC 值
int readMCP3008(byte channel) {
    if (channel < 0 || channel > 7) return -1;  // 通道范围校验

    // 配置 SPI 参数：1MHz 时钟，MSB 在前，SPI 模式 0
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

    digitalWrite(CS_PIN, LOW);  // 开始通信：拉低片选

    SPI.transfer(0x01);                           // 发送起始位
    byte highByte = SPI.transfer((channel << 4) | 0x80);  // 包含通道信息和单端输入标志
    byte lowByte  = SPI.transfer(0x00);                  // 补齐时序，获取剩余数据

    digitalWrite(CS_PIN, HIGH); // 结束通信：拉高片选
    SPI.endTransaction();       // 释放 SPI 总线配置

    // 提取有效位：highByte 的低 2 位 + lowByte 的全部 8 位 = 10 位 ADC 值
    return ((highByte & 0x03) << 8) | lowByte;
}

void loop() {
    for (int i = 0; i < 8; i++) {
        int adcValue = readMCP3008(i);
        float voltage = adcValue * (3.3 / 1023.0);  // 转换为实际电压（10 位分辨率）

        Serial.printf("通道 %d: ADC=%d, 电压=%.2fV\n", i, adcValue, voltage);
    }

    Serial.println("------------------------");
    delay(1000);  // 每秒刷新一次
}
