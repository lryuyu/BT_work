#ifndef SENSORFILTER_H
#define SENSORFILTER_H
#include <Arduino.h>
#include "config.h"
#include "MCP3008.h"

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
    uint16_t pp_0;
    uint16_t pp_1;
    uint16_t head_0;
    uint16_t nose_0;
    uint16_t nose_1;
    uint16_t head_1;
    uint16_t chin_0;
    uint16_t AF;
    uint16_t DIS;
    uint16_t AED_0;
    uint16_t AED_1;

}sensor_data_t;

// 增量滤波状态结构体（替代原全局数组，每个通道独立）
struct SimpleFilter {
    uint16_t buffer[(int)FILTER_MAX] = {0}; // 滤波窗口缓存
    uint8_t index = 0;                      // 当前缓存索引
    uint32_t sum = 0;                       // 缓存值累加和（增量更新）
    uint8_t filled = 0;                     // 已填充的缓存数量
};

// 全局声明：滤波后数据的互斥锁（保护多任务访问）
extern SemaphoreHandle_t sensorDataMutex;


void sensor_init();

uint16_t sensor_read_DIS();

uint16_t sensor_read_AF();

void TCA9548_select_channel(uint8_t channel);

void PN532_init();

uint32_t PN532_1_read_uid();

uint32_t PN532_2_read_uid();

uint16_t sensor_read_raw(adc_channel_t ch);

void sensor_read_all_raw(sensor_data_t *data);

void sensor_read_all_filtered(sensor_data_t *data);

#endif