#include "MCP3008.h"
// #include <io_pin_remap.h>
#include <SPI.h>
#include "config.h"

void MCP3008_init() {
    // ??????????
    pinMode(MCP3008_CS0, OUTPUT);
    pinMode(MCP3008_CS1, OUTPUT);

    digitalWrite(MCP3008_CS0, HIGH);
    digitalWrite(MCP3008_CS1, HIGH);

    // ????? SPI
    SPI.begin();
    delay(1000);
}

uint16_t MCP3008_read(const adc_id_t adc_id,uint8_t channel) {
    if (channel < 0 || channel > 7) return -1;

    SPI.beginTransaction(SPISettings(1000000,MSBFIRST,SPI_MODE0));

    digitalWrite(MCP3008_CS0, LOW);
    digitalWrite(MCP3008_CS1, LOW);

    SPI.transfer(0x01);

    byte highByte = SPI.transfer((0x08 | channel) << 4 | 0x80);

    byte lowByte = SPI.transfer(0x00);

    digitalWrite(MCP3008_CS0, HIGH);
    digitalWrite(MCP3008_CS1, LOW);

    return ((highByte & 0x03) << 8) | lowByte;

}


