#include <cstdint>

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

//SPI????
#define SPI_SCLK 48
#define SPI_MOSI 38
#define SPI_MISO 47

//MCP3008
#define MCP3008_CS0 21//D10
#define MCP3008_CS1 18 //D9

//I2C
#define I2C_SDA 4 //11
#define I2C_SCL 5 //12
#define TCA9548_ADDR 0x70

#define TCA9548_ADDR 0x70

//????
#define A_SENS_DIS_EA0_PIN 46 //A0
#define A_SEND_AF_EA1_PIN 1 //A1

#define BLE_SERVICE_UUID  "eff5d318-add9-4b4e-afc1-7067c5f6a802"
#define BLE_CHARACTERISTIC_UUID  "ea6d2750-a60f-436c-a60c-437e43c7a32b"
#define BLE_ADV_APPEARANCE      0x0540
#define BLE_CONFIG_NAME  "EPHW:MSTESCPR121314"

typedef enum {
    ADC_1 = 0,
    ADC_2 = 1
}adc_id_t;

typedef struct {
    adc_id_t adc;
    uint8_t ch;
}adc_channel_t;

//??????ADC??????
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
#define SEND_INTERVAL 1000

#endif