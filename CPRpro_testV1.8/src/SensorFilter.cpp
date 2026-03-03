#include "SensorFilter.h"
#include "config.h"
#include <cstring>
#include "freertos/task.h"
#include <Wire.h>
#include <Adafruit_PN532.h>
#define PN532_IRQ 14
#define PN532_RESET 17

Adafruit_PN532 PN532(PN532_IRQ, PN532_RESET);
const uint8_t TARGET_UID_0[TARGET_UID_LEN] = {0x12, 0x34, 0x56, 0x78};
const uint8_t TARGET_UID_1[TARGET_UID_LEN] = {0xAB, 0xCD, 0xEF, 0x01};

static sensor_data_t s_filteredData = {0};

static SimpleFilter s_filterStates[11] ;
//static SimpleFilter s_filterStates[15] ;

SemaphoreHandle_t sensorDataMutex = nullptr;

//????????
static uint16_t simpleFilter(const uint16_t newVal, SimpleFilter& state) {

    const uint16_t oldVal = state.buffer[state.index];
    state.sum = state.sum - oldVal + newVal;

    state.buffer[state.index] = newVal;
    state.index = (state.index + 1) % (int)FILTER_MAX;

    if (state.filled < (int)FILTER_MAX) state.filled++;

    return state.sum / state.filled;
}

//?????????????
 static void batchSensorTask(void *pvParameters) {
    sensor_data_t rawData = {0};
    while (true) {

        sensor_read_all_raw(&rawData);

        rawData.timestamp = millis();

        rawData.lp_1 = simpleFilter(rawData.lp_1 , s_filterStates[0]);
        rawData.lp_2 = simpleFilter(rawData.lp_2 , s_filterStates[1]);
        rawData.lp_3 = simpleFilter(rawData.lp_3 , s_filterStates[2]);
        rawData.lp_4 = simpleFilter(rawData.lp_4 , s_filterStates[3]);
        rawData.lp_5 = simpleFilter(rawData.lp_5 , s_filterStates[4]);
        // rawData.lp_6 = simpleFilter(rawData.lp_6 , s_filterStates[5]);
        // rawData.lp_7 = simpleFilter(rawData.lp_7 , s_filterStates[6]);
        // rawData.lp_8 = simpleFilter(rawData.lp_8 , s_filterStates[7]);
        rawData.pp_L = simpleFilter(rawData.pp_L , s_filterStates[5]);
        rawData.pp_R = simpleFilter(rawData.pp_R , s_filterStates[6]);
        rawData.head_0 = simpleFilter(rawData.head_0 , s_filterStates[7]);
        rawData.nose_L = simpleFilter(rawData.nose_L, s_filterStates[8]);
        rawData.nose_R = simpleFilter(rawData.nose_R, s_filterStates[9]);
        rawData.chin_0 = simpleFilter(rawData.chin_0, s_filterStates[10]);


        if (xSemaphoreTake(sensorDataMutex, (TickType_t)10) == pdTRUE) {
            memcpy(&s_filteredData, &rawData, sizeof(sensor_data_t));
            xSemaphoreGive(sensorDataMutex);
        }

        //  2ms???
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

//AF DIS begin
int sensor_init() {
    MCP3008_init();

    pinMode(A_SENS_DIS_EA0_PIN, INPUT);
    pinMode(A_SEND_AF_EA1_PIN, INPUT);


    memset(s_filterStates, 0, sizeof(s_filterStates));

    sensorDataMutex = xSemaphoreCreateMutex();


    xTaskCreate(
        batchSensorTask,
        "BatchSensorTask",
        4096,
        nullptr,
        5,
        nullptr
    );
    return 0;
}

//?ADC?
uint16_t sensor_read_raw(const adc_channel_t ch) {
    return MCP3008_read(ch.adc, ch.ch);
}

//?DIS?
uint16_t sensor_read_DIS() {
    return analogRead(A_SENS_DIS_EA0_PIN);
}

//?AF?
uint16_t sensor_read_AF() {
    return analogRead(A_SEND_AF_EA1_PIN);
}

// ???TCA9548???
void TCA9548_select_channel(uint8_t channel) {
    if (channel > 7) return;
    Wire.beginTransmission(TCA9548_ADDR);
    Wire.write(1 << channel);
    Wire.endTransmission();
    delay(10);
}

// PN532 begin
void PN532_init() {
    Wire.begin(I2C_SDA, I2C_SCL);
    TCA9548_select_channel(0);
    if (PN532.begin()) {
        PN532.SAMConfig();
        Serial.println("PN532_L Config");
    }else {
        Serial.println("PN532_L Failed");
    }

    TCA9548_select_channel(1);
    if (PN532.begin()) {
        PN532.SAMConfig();
        Serial.println("PN532_R Config");
    }else {
        Serial.println("PN532_R Failed");
    }

    Wire.begin(I2C_SDA, I2C_SCL);
}

// 第一个PN532读取并对比UID
uint32_t PN532_1_read_uid() {
    TCA9548_select_channel(0);
    uint8_t uid[7];
    uint8_t uidLength;

    //超时时间50ms
    if (PN532.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength,100)) {

        if (uidLength != TARGET_UID_LEN) return 0;
        for (uint8_t i = 0; i < TARGET_UID_LEN; i++) {
            if (uid[i] != TARGET_UID_0[i]) return 0;
        }
        return 1;
    }
    return 2;
}

// 第二个PN532读取并对比UID
uint32_t PN532_2_read_uid() {
    TCA9548_select_channel(1); // 切换到通道1，对应第二个PN532
    uint8_t uid[7];
    uint8_t uidLength;

    if (PN532.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength,100)) {
        if (uidLength != TARGET_UID_LEN) return 0; // 长度不匹配，返回0
        for (uint8_t i = 0; i < TARGET_UID_LEN; i++) {
            if (uid[i] != TARGET_UID_1[i]) return 0; // 任意字节不匹配，返回0
        }
        return 1; // 所有字节都匹配，返回1
    }
    return 2; // 读取失败，返回2
}


void sensor_read_all_raw(sensor_data_t *data) {
    if (data == nullptr) return;


    data->lp_1 = MCP3008_read(A_SENS_LP_1_PIN.adc,A_SENS_LP_1_PIN.ch);
    data->lp_2 = MCP3008_read(A_SENS_LP_2_PIN.adc,A_SENS_LP_2_PIN.ch);
    data->lp_3 = MCP3008_read(A_SENS_LP_3_PIN.adc,A_SENS_LP_3_PIN.ch);
    data->lp_4   = MCP3008_read(A_SENS_LP_4_PIN.adc,A_SENS_LP_4_PIN.ch);
    data->lp_5   = MCP3008_read(A_SENS_LP_5_PIN.adc,A_SENS_LP_5_PIN.ch);
    // data->lp_6 = sensor_read_raw(A_SENS_LP_6_PIN);
    // data->lp_7 = sensor_read_raw(A_SENS_LP_7_PIN);
    // data->lp_8 = sensor_read_raw(A_SENS_LP_8_PIN);
    // data->lp_9 = sensor_read_raw(A_SENS_LP_9_PIN);
    data->pp_L   = MCP3008_read(A_SENS_PP_0_PIN.adc,   A_SENS_PP_0_PIN.ch);
    data->pp_R   = MCP3008_read(A_SENS_PP_1_PIN.adc,   A_SENS_PP_1_PIN.ch);
    data->head_0 = MCP3008_read(A_SENS_HEAD_0_PIN.adc, A_SENS_HEAD_0_PIN.ch);
    data->nose_L = MCP3008_read(A_SENS_NOSE_0_PIN.adc, A_SENS_NOSE_0_PIN.ch);
    data->nose_R = MCP3008_read(A_SENS_NOSE_1_PIN.adc, A_SENS_NOSE_1_PIN.ch);
    data->chin_0 = MCP3008_read(A_SENS_CHIN_0_PIN.adc, A_SENS_CHIN_0_PIN.ch);
    data->DIS    = sensor_read_DIS();
    data->AF     = sensor_read_AF();
}

//
void sensor_read_all_filtered(sensor_data_t *data) {
    if (data == nullptr || sensorDataMutex == nullptr) return;

    if (xSemaphoreTake(sensorDataMutex, (TickType_t)50) == pdTRUE) {
        memcpy(data, &s_filteredData, sizeof(sensor_data_t));
        xSemaphoreGive(sensorDataMutex);
    } else {

        sensor_read_all_raw(data);
    }
}

// Sensor task
[[noreturn]] void sensorTask(void *pvParameters) {
    auto* sensor = static_cast<SensorConfig *>(pvParameters);//获取传感器配置
    while (true) {
        const int val = simpleFilter(sensor->pin, sensor->filterState);//获取互斥锁，更新最新值
        if (xSemaphoreTake(sensor->mutex, (TickType_t)10) == pdTRUE) {
            sensor->latestValue = val;
            xSemaphoreGive(sensor->mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}