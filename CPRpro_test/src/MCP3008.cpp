#include "MCP3008.h"
#include <io_pin_remap.h>
#include <SPI.h>
#include "config.h"

void MCP3008_init() {
    // 配置SPI硬件引脚
    pinMode(SPI_SCLK,OUTPUT);  // SPI时钟引脚
    pinMode(SPI_MOSI,OUTPUT);  // SPI主机输出/从机输入
    pinMode(SPI_MISO,INPUT);   // SPI主机输入/从机输出

    // 配置MCP3008片选引脚
    pinMode(MCP3008_CS0,OUTPUT); // ADC_1片选
    pinMode(MCP3008_CS1,OUTPUT); // ADC_2片选

    // 片选引脚默认高电平（不选中芯片）
    digitalWrite(MCP3008_CS0,HIGH);
    digitalWrite(MCP3008_CS1,HIGH);

    // 初始化SPI总线
    SPI.begin();
    // 设置SPI时钟分频（系统时钟/16，保证通信稳定性）
    SPI.setClockDivider(SPI_CLOCK_DIV16);
}

uint16_t MCP3008_read(const adc_id_t adc_id,uint8_t channel) {
    if (channel > 7) return 0;

    const uint8_t cs_pin = (adc_id == ADC_1) ? MCP3008_CS0 : MCP3008_CS1;

    digitalWrite(cs_pin, LOW);

    uint8_t cmd = 0x01;
    cmd |= (0x80 | channel )<< 4;

    SPI.transfer(cmd);

    uint16_t val = SPI.transfer(0x00) << 2;

    val |= SPI.transfer(0x00) >> 6;

    digitalWrite(cs_pin, HIGH);

    return val;
}