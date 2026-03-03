#include "MCP3008.h"
// #include <io_pin_remap.h>
#include <SPI.h>
#include "config.h"

void MCP3008_init() {
    // 设置片选引脚
    pinMode(MCP3008_CS0, OUTPUT);
    pinMode(MCP3008_CS1, OUTPUT);

    digitalWrite(MCP3008_CS0, HIGH);
    digitalWrite(MCP3008_CS1, HIGH);

    // 初始化 SPI
    SPI.begin();
}

uint16_t MCP3008_read(const adc_id_t adc_id,uint8_t channel) {
    if (channel > 7) return 0;

    const uint8_t cs_pin = (adc_id == ADC_1) ? MCP3008_CS0 : MCP3008_CS1;
    SPI.beginTransaction(SPISettings(1000000,MSBFIRST,SPI_MODE0));

    digitalWrite(cs_pin, LOW);
    SPI.transfer(0x01);

    uint8_t high = SPI.transfer((0x08 | channel) << 4);

    uint8_t low  = SPI.transfer(0x00);

    digitalWrite(cs_pin, HIGH);

    SPI.endTransaction();

    return ((high & 0x03) << 8) | low;

}


