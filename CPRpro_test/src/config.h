#include <cstdint>

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

//SPI引脚
#define SPI_SCLK 14//D13
#define SPI_MOSI 11//D11
#define SPI_MISO 12//D12

//MCP3008
#define MCP3008_CS0 10//D10
#define MCP3008_CS1 9 //D9

//I2C
#define I2C_SDA 48 //D2
#define I2C_SCL 47 //D3
#define TCA9548_ADDR 0x70

//数字
#define A_SENS_DIS_EA0_PIN 46 //D4
#define A_SEND_AF_EA1_PIN 45 //D5

#define BLE_SERVICE_UUID  "3001942d-bb32-47d2-9f8d-9bca93041212"
#define BLE_CHARACTERISTIC_UUID  "ae8a588d-0e64-4f95-b4c5-c134d27747a3"
#define BLE_ADV_APPEARANCE      0x0540
#define BLE_CONFIG_NAME  "EPHW:MSTESCPR12345"

typedef enum {
    ADC_1 = 0,
    ADC_2 = 1
}adc_id_t;

typedef struct {
    adc_id_t adc;
    uint8_t ch;
}adc_channel_t;

//传感器ADC通道映射
#define A_SENS_LP_1_PIN  (adc_channel_t){ADC_1,0}
#define A_SENS_LP_2_PIN  (adc_channel_t){ADC_1,1}
#define A_SENS_LP_3_PIN  (adc_channel_t){ADC_1,2}
#define A_SENS_LP_4_PIN  (adc_channel_t){ADC_1,3}
#define A_SENS_LP_5_PIN  (adc_channel_t){ADC_1,4}
// #define A_SENS_LP_6_PIN  (adc_channel_t){ADC_1,5}
// #define A_SENS_LP_7_PIN  (adc_channel_t){ADC_1,6}
// #define A_SENS_LP_8_PIN  (adc_channel_t){ADC_1,7}
// #define A_SENS_LP_9_PIN  (adc_channel_t){ADC_2,0}
#define A_SENS_PP_0_PIN  (adc_channel_t){ADC_2, 1}
#define A_SENS_PP_1_PIN  (adc_channel_t){ADC_2, 2}
#define A_SENS_HEAD_0_PIN  (adc_channel_t){ADC_2, 3}
#define A_SENS_NOSE_0_PIN  (adc_channel_t){ADC_2, 4}
#define A_SENS_NOSE_1_PIN  (adc_channel_t){ADC_2, 5}
#define A_SENS_CHIN_0_PIN  (adc_channel_t){ADC_2, 6}

#define  FILTER_MAX 100
#define WIFI_RECONNECT_INTERVAL 5000
#define SOCKET_RECONNECT_INTERVAL 5000
#define SEND_INTERVAL 100

#endif