#ifndef SENSORFILTER_H
#define SENSORFILTER_H
#include <Arduino.h>
#include "config.h"
#include "MCP3008.h"



#define TARGET_UID_LEN 4
extern const uint8_t TARGET_UID_0[TARGET_UID_LEN];
extern const uint8_t TARGET_UID_1[TARGET_UID_LEN];

typedef struct {
    uint16_t timestamp;
    uint16_t lp_1;
    uint16_t lp_2;
    uint16_t lp_3;
    uint16_t lp_4;
    uint16_t lp_5;
    uint16_t lp_6;
    uint16_t lp_7;
    uint16_t lp_8;
    uint16_t lp_9;
    uint16_t pp_L;
    uint16_t pp_R;
    uint16_t head_0;
    uint16_t nose_L;
    uint16_t nose_R;
    uint16_t head_R;
    uint16_t chin_0;
    uint16_t AF;
    uint16_t DIS;
    uint16_t AED_L;
    uint16_t AED_R;

}sensor_data_t;

//?????
struct SimpleFilter {
    uint16_t buffer[(int)FILTER_MAX] = {0};
    uint8_t index = 0;
    uint32_t sum = 0;
    uint8_t filled = 0;
};

struct SensorConfig {
    int pin;
    SimpleFilter filterState;
    volatile int latestValue;
    SemaphoreHandle_t mutex;
};


extern SemaphoreHandle_t sensorDataMutex;


int sensor_init();

uint16_t sensor_read_DIS();

uint16_t sensor_read_AF();

void TCA9548_select_channel(uint8_t channel);

void PN532_init();

uint32_t PN532_1_read_uid();

uint32_t PN532_2_read_uid();

uint16_t sensor_read_raw(adc_channel_t ch);

void sensor_read_all_raw(sensor_data_t *data);

void sensor_read_all_filtered(sensor_data_t *data);

bool isTargetNFCUID(const uint8_t* uid, uint8_t uid_len);

#endif