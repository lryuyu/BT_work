#include "MCP3008.h"
#include <io_pin_remap.h>
#include <SPI.h>
#include "config.h"

void MCP3008_init() {
    // 初始SPI
    pinMode(SPI_SCLK,OUTPUT);  // SPI???????
    pinMode(SPI_MOSI,OUTPUT);  // SPI???????/???????
    pinMode(SPI_MISO,INPUT);   // SPI????????/??????

    // 初始片选MCP3008
    pinMode(MCP3008_CS0,OUTPUT); // ADC_1??
    pinMode(MCP3008_CS1,OUTPUT); // ADC_2??

    digitalWrite(MCP3008_CS0,HIGH);
    digitalWrite(MCP3008_CS1,HIGH);

    // 启动SPI
    SPI.begin();
    //降低时钟
    SPI.setClockDivider(SPI_CLOCK_DIV16);
}

uint16_t MCP3008_read(const adc_id_t adc_id,uint8_t channel) {
    if (channel > 7) return 0;

    const uint8_t cs_pin = (adc_id == ADC_1) ? MCP3008_CS0 : MCP3008_CS1;

    digitalWrite(cs_pin, LOW);
    //发送读取命令
    uint8_t cmd = 0x01;
    cmd |= (0x80 | channel )<< 4;

    SPI.transfer(cmd);
    //读取10位数据
    uint16_t val = SPI.transfer(0x00) << 2;

    val |= SPI.transfer(0x00) >> 6;
    //取消选中
    digitalWrite(cs_pin, HIGH);

    return val;
}