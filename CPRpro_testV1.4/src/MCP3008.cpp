#include "MCP3008.h"
#include <io_pin_remap.h>
#include <SPI.h>
#include "config.h"

void MCP3008_init() {
    // ???SPI
    pinMode(SPI_SCLK,OUTPUT);  // SPI???????
    pinMode(SPI_MOSI,OUTPUT);  // SPI???????/???????
    pinMode(SPI_MISO,INPUT);   // SPI????????/??????

    // ?????MCP3008
    pinMode(MCP3008_CS0,OUTPUT); // ADC_1??
    pinMode(MCP3008_CS1,OUTPUT); // ADC_2??

    digitalWrite(MCP3008_CS0,HIGH);
    digitalWrite(MCP3008_CS1,HIGH);

    // ????SPI
    SPI.begin();
    //???????
    SPI.setClockDivider(SPI_CLOCK_DIV16);
}

uint16_t MCP3008_read(const adc_id_t adc_id,uint8_t channel) {
    if (channel < 0 ||channel > 7) return 0;

    const uint8_t cs_pin = (adc_id == ADC_1) ? MCP3008_CS0 : MCP3008_CS1;

    digitalWrite(cs_pin, LOW);
    //起始位1, 单端模式1, 通道号(3位)左移4位
    uint8_t command = 0x01 | (channel << 4);

    SPI.transfer(command);
    uint8_t high = SPI.transfer(0);
    uint8_t low  = SPI.transfer(0);
    digitalWrite(cs_pin, HIGH);

    // high的低2位是10位结果的高2位，low是低8位
    return ((high & 0x03) << 8) | low;
}


