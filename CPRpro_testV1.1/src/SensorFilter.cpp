#include "SensorFilter.h"
#include "config.h"
#include <cstring>
#include "freertos/task.h"
#include <Wire.h>
#include <Adafruit_PN532.h>
#define PN532_IRQ 0
#define PN532_RESET 7

Adafruit_PN532 PN532(PN532_IRQ, PN532_RESET);

static sensor_data_t s_filteredData = {0};

static SimpleFilter s_filterStates[11] ;
//static SimpleFilter s_filterStates[15] ;

SemaphoreHandle_t sensorDataMutex = nullptr;

//简单滑动滤波
static uint16_t simpleFilter(uint16_t newVal, SimpleFilter& state) {

    const uint16_t oldVal = state.buffer[state.index];
    state.sum = state.sum - oldVal + newVal;

    state.buffer[state.index] = newVal;
    state.index = (state.index + 1) % (int)FILTER_MAX;

    if (state.filled < (int)FILTER_MAX) state.filled++;

    return state.sum / state.filled;
}

//传感器批量读取
[[noreturn]] static void batchSensorTask(void *pvParameters) {
    sensor_data_t rawData = {0};
    while (true) {

        sensor_read_all_raw(&rawData);

        rawData.timestamp = millis();

        uint16_t* rawFields = &rawData.lp_1;

        //对11个核心通道滤波
        for (int i = 0; i < 11; i++) {  //for (int i = 0; i < 15; i++)原本15个传感器，现在减少四个
            rawFields[i] = simpleFilter(rawFields[i], s_filterStates[i]);
        }

        //线程安全更新滤波后的数据
        if (xSemaphoreTake(sensorDataMutex, (TickType_t)10) == pdTRUE) {
            memcpy(&s_filteredData, &rawData, sizeof(sensor_data_t));
            xSemaphoreGive(sensorDataMutex);
        }

        //  2ms间隔
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

//传感器初始化
void sensor_init() {
    MCP3008_init();

    //初始化数字传感器引脚
    pinMode(A_SENS_DIS_EA0_PIN, OUTPUT);
    pinMode(A_SEND_AF_EA1_PIN, OUTPUT);
    digitalWrite(A_SENS_DIS_EA0_PIN, HIGH);
    digitalWrite(A_SEND_AF_EA1_PIN, LOW);

    //初始化滤波缓冲区
    memset(s_filterStates, 0, sizeof(s_filterStates));

    //创建互斥锁
    sensorDataMutex = xSemaphoreCreateMutex();

    //创建传感器读取任务
    xTaskCreate(
        batchSensorTask,
        "BatchSensorTask",
        2048,
        nullptr,
        5,
        nullptr
    );
}

//读取原始ADC数据
uint16_t sensor_read_raw(const adc_channel_t ch) {
    return MCP3008_read(ch.adc, ch.ch);
}

//读取DIS数字传感器
uint16_t sensor_read_DIS() {
    digitalWrite(A_SENS_DIS_EA0_PIN, HIGH);
    delay(10);
    uint16_t value =  digitalRead(A_SENS_DIS_EA0_PIN);
    digitalWrite(A_SENS_DIS_EA0_PIN, LOW);
    return value;
}

//读取AF数字传感器
uint16_t sensor_read_AF() {
    digitalWrite(A_SEND_AF_EA1_PIN, HIGH);
    delay(10);
    uint16_t value =  digitalRead(A_SEND_AF_EA1_PIN);
    digitalWrite(A_SEND_AF_EA1_PIN, LOW);
    return value;
}

// 选择TCA9548通道
void TCA9548_select_channel(uint8_t channel) {
    if (channel > 7) return;
    Wire.beginTransmission(TCA9548_ADDR);
    Wire.write(1 << channel);
    Wire.endTransmission();
}

// PN532初始化
void PN532_init() {
    Wire.begin(I2C_SDA, I2C_SCL);
    if (!PN532.begin()) {
        while (false);
    }
    PN532.setPassiveActivationRetries(0xFF);
    PN532.SAMConfig();
}

// 读取PN532 UID
uint32_t PN532_1_read_uid() {
    TCA9548_select_channel(0);
    uint8_t uid[7];
    uint8_t uidLength;
    if (PN532.readPassiveTargetID(PN532_MIFARE_ISO14443A,uid,&uidLength)) {
        return (uid[0] << 24) | (uid[1] << 16) | (uid[2] << 8) | uid[3];
    }
    return 0;
}

uint32_t PN532_2_read_uid() {
    TCA9548_select_channel(1);
    uint8_t uid[7];
    uint8_t uidLength;
    if (PN532.readPassiveTargetID(PN532_MIFARE_ISO14443A,uid,&uidLength)) {
        return (uid[0] << 24) | (uid[1] << 16) | (uid[2] << 8) | uid[3];
    }
    return 0;
}

// 读取所有原始传感器数据
void sensor_read_all_raw(sensor_data_t *data) {
    if (data == nullptr) return;


    data->lp_1 = sensor_read_raw(A_SENS_LP_1_PIN);
    data->lp_2 = sensor_read_raw(A_SENS_LP_2_PIN);
    data->lp_3 = sensor_read_raw(A_SENS_LP_3_PIN);
    data->lp_4 = sensor_read_raw(A_SENS_LP_4_PIN);
    data->lp_5 = sensor_read_raw(A_SENS_LP_5_PIN);
    // data->lp_6 = sensor_read_raw(A_SENS_LP_6_PIN);
    // data->lp_7 = sensor_read_raw(A_SENS_LP_7_PIN);
    // data->lp_8 = sensor_read_raw(A_SENS_LP_8_PIN);
    // data->lp_9 = sensor_read_raw(A_SENS_LP_9_PIN);
    data->pp_0 = sensor_read_raw(A_SENS_PP_0_PIN);
    data->pp_1 = sensor_read_raw(A_SENS_PP_1_PIN);
    data->head_0 = sensor_read_raw(A_SENS_HEAD_0_PIN);
    data->nose_0 = sensor_read_raw(A_SENS_NOSE_0_PIN);
    data->nose_1 = sensor_read_raw(A_SENS_NOSE_1_PIN);
    data->chin_0 = sensor_read_raw(A_SENS_CHIN_0_PIN);
    data->head_1 = 0;
    data->DIS = sensor_read_DIS();
    data->AF = sensor_read_AF();
    data->AED_0 = PN532_1_read_uid();
    data->AED_1 = PN532_2_read_uid();
}

// 读取滤波后的数据
void sensor_read_all_filtered(sensor_data_t *data) {
    if (data == nullptr || sensorDataMutex == nullptr) return;

    if (xSemaphoreTake(sensorDataMutex, (TickType_t)50) == pdTRUE) {
        memcpy(data, &s_filteredData, sizeof(sensor_data_t));
        xSemaphoreGive(sensorDataMutex);
    } else {
        // 锁获取失败时读取原始数据
        sensor_read_all_raw(data);
    }
}