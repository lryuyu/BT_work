#ifndef MCP3008_H
#define MCP3008_H
#include <cstdint>
#include "config.h"

void MCP3008_init();

uint16_t MCP3008_read(adc_id_t adc_id, uint8_t channel);

#endif //MCP3008_H