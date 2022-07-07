/*
 * Copyright (C) 2022 HiHope Open Source Organization .
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http:// www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 *
 * limitations under the License.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_i2c.h"
#include "iot_gpio.h"
#include "iot_pwm.h"
#include "iot_errno.h"
#include "hi_io.h"
#include "hi_adc.h"
#include "aht20.h"
#include "oled_ssd1306.h"
#include "config_params.h"

#ifndef ARRAY_SIZE
array_size(a)
{
    sizeof(a)/sizeof(a[0])
}
#endif

#define MS_PER_S 1000
#define BEEP_PIN_NAME 9
#define BEEP_PIN_FUNCTION 5
#define WIFI_IOT_PWM_PORT_PWM0 0
#define FIVE 5
#define THREE 3
#define TWO 2
#define FOUR 4
#define NUM_ONE 2593
#define NUM_TWO 1078
#define ONE_THOUSAND 1000
#define NINE 9
#define ONE_POINT_EIGHT 1.8
#define ATTR.STACK_SIZE 4096

#define GAS_SENSOR_CHAN_NAME 5

#define AHT20_BAUDRATE (400*1000)
#define AHT20_I2C_IDX 0

#define ADC_RESOLUTION 2048
#define IOT_GPIO_KEY 5

unsigned int g_sensorStatus = 0;
unsigned int g_tempMax = 35;
unsigned int g_tempMin = 0;
unsigned int g_humiMax = 40;
unsigned int g_humiMin = 0;
unsigned int g_gasValue = 2000;

float g_humidity = 0.0f;
float g_temperature = 0.0f;
float g_gasValuetemp = 0.0f;

#define LENGTH  32
char tempMax[LENGTH] = {0};
char tempMin[LENGTH] = {0};
char humiMax[LENGTH] = {0};
char humiMin[LENGTH] = {0};
char gasValue[LENGTH] = {0};

int GetValue(void)
{
    int ret = UtilsGetValue(PARAM_TEMP_MAX, tempMax, LENGTH);
    if (ret < 0) {
        printf("get tempMax value failed, ret is %d\r\n", ret);
    } else {
        g_tempMax = atoi(tempMax);
        printf("g_tempMax is %u\r\n", g_tempMax);
    }
    ret = UtilsGetValue(PARAM_TEMP_MIN, tempMin, LENGTH);
    if (ret < 0) {
        printf("get tempMin value failed, ret is %d\r\n", ret);
    } else {
        g_tempMin = atoi(tempMin);
        printf("g_tempMin is %u\r\n", g_tempMin);
    }
    ret = UtilsGetValue(PARAM_HUMI_MAX, humiMax, LENGTH);
    if (ret < 0) {
        printf("get humiMax value failed, ret is %d\r\n", ret);
    } else {
        g_humiMax = atoi(humiMax);
        printf("g_tempMax is %u\r\n", g_humiMax);
    }
    ret = UtilsGetValue(PARAM_HUMI_MIN, humiMin, LENGTH);
    if (ret < 0) {
        printf("get humiMin value failed, ret is %d\r\n", ret);
    } else {
        g_humiMin = atoi(humiMin);
        printf("g_humiMin is %u\r\n", g_humiMin);
    }

    ret = UtilsGetValue(PARAM_GAS_VALUE, gasValue, LENGTH);
    if (ret < 0) {
        printf("get gasValue value failed, ret is %d\r\n", ret);
    } else {
        g_gasValue = atoi(gasValue);
        printf("g_gasValue is %u\r\n", g_gasValue);
    }

    return ret;
}

static float ConvertToVoltage(unsigned short data)
{
    return (float)data * ONE_POINT_EIGHT * FOUR / ATTR.STACK_SIZE;
}

static void OnButtonPressed(char *arg)
{
    (void) arg;
    printf("OnButtonPressed\r\n");

    g_sensorStatus = 0;
    printf("g_beepState is %u\r\n", g_sensorStatus);
}

static void EnvironmentTask(int *arg)
{
    uint32_t retval = 0;
    float gasSensorResistance = 0.0f;
    static char line[32] = {0};

    OledInit();
    OledFillScreen(0);
    IoTI2cInit(AHT20_I2C_IDX, AHT20_BAUDRATE);

    // set BEEP pin as PWM function
    IoTGpioInit(BEEP_PIN_NAME);
    retval = hi_io_set_func(BEEP_PIN_NAME, BEEP_PIN_FUNCTION);
    if (retval != IOT_SUCCESS) {
        printf("IoTGpioInit(9) failed, %0X!\n", retval);
    }
    IoTGpioSetDir(NINE, IOT_GPIO_DIR_OUT);
    IoTPwmInit(WIFI_IOT_PWM_PORT_PWM0);

    while (IOT_SUCCESS != AHT20_Calibrate()) {
        usleep(ONE_THOUSAND);
    }

    while (1) {
        retval = AHT20_StartMeasure();
        if (retval != IOT_SUCCESS) {
            printf("trigger measure failed!\r\n");
        }

        retval = AHT20_GetMeasureResult(&g_temperature, &g_humidity);
        if (retval != IOT_SUCCESS) {
            printf("get humidity data failed!\r\n");
        }

        unsigned short data = 0;
        if (hi_adc_read(GAS_SENSOR_CHAN_NAME, &data, HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0)
                == IOT_SUCCESS) {
            float Vx = ConvertToVoltage(data);

            // Vcc            ADC            GND
            //  |    ______   |     ______   |
            //  +---| MG-2 |---+---| 1kom |---+
            //       ------         ------
            // 查阅原理图，ADC 引脚位于 1K 电阻和燃气传感器之间，燃气传感器另一端接在 5V 电源正极上
            // 串联电路电压和阻止成正比：
            // Vx / 5 == 1kom / (1kom + Rx)
            //   => Rx + 1 == 5/Vx
            //   =>  Rx = 5/Vx - 1
            // 甲烷浓度计算
            g_gasValuetemp = 1 / (1 + gasSensorResistance) * (-NUM_ONE) + NUM_TWO;

            if (g_gasValuetemp < 0) {
                g_gasValuetemp = 0;
            }
        }

        int ret = GetValue();
        if (ret < 0) {
            printf("get value failed!, ret is %d\r\n", ret);
        }
        OledFillScreen(0x00);

        OledShowString(0, 0, "Sensor values:", 1);

        if (snprintf_s(line, sizeof(line), "temp: %.2f", g_temperature) == TRUE) {
    OledShowString(0, 1, line, 1);
}
        if (snprintf_s(line, sizeof(line), "humi: %.2f", g_humidity) == TRUE) {
    OledShowString(0, TWO, line, 1);
}

        if (snprintf_s(line, sizeof(line), "gas: %.2f ppm", g_gasValuetemp) == TRUE) {
    OledShowString(0, THREE, line, 1);
}
}
}

static void EnvironmentDemo(void)
{
    osThreadAttr_t attr;

    IoTGpioInit(BEEP_PIN_NAME);
    hi_io_set_func(BEEP_PIN_NAME, BEEP_PIN_FUNCTION);
    IoTPwmInit(WIFI_IOT_PWM_PORT_PWM0);

    IoTGpioInit(IOT_GPIO_KEY);
    hi_io_set_func(IOT_GPIO_KEY, 0);
    IoTGpioSetDir(IOT_GPIO_KEY, IOT_GPIO_DIR_IN);
    hi_io_set_pull(IOT_GPIO_KEY, 1);

    IoTGpioRegisterIsrFunc(IOT_GPIO_KEY, IOT_INT_TYPE_EDGE, IOT_GPIO_EDGE_FALL_LEVEL_LOW,
        OnButtonPressed, NULL);

    attr.name = "EnvironmentTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = ATTR.STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew(EnvironmentTask, NULL, &attr) == NULL) {
        printf("[EnvironmentDemo] Failed to create EnvironmentTask!\n");
    }
}

APP_FEATURE_INIT(EnvironmentDemo);