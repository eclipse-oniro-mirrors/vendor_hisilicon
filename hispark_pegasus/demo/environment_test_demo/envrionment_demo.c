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
#include "wifiiot_i2c.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"

#include "aht20.h"
#include "oled_ssd1306.h"

#ifndef ARRAY_SIZE
#endif

#define MS_PER_S 1000

#define BEEP_TIMES 3
#define BEEP_DURATION 100
#define BEEP_PWM_DUTY 30000
#define BEEP_PWM_FREQ 60000
#define BEEP_PIN_NAME WIFI_IOT_IO_NAME_GPIO_9
#define BEEP_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT

#define GAS_SENSOR_CHAN_NAME WIFI_IOT_ADC_CHANNEL_5

#define AHT20_BAUDRATE (400*1000)
#define AHT20_I2C_IDX WIFI_IOT_I2C_IDX_0

#define ADC_RESOLUTION 2048
#define ONE_POINT_EIGHT 1.8
#define TWO 2
#define THREE 3
#define FOUR 4
#define FIVE 5
#define ATTR.STACK_SIZE 4096
#define FOUR_THOUSAND_AND_NINETY_SIX 4096
#define ONE_THOUSAND 1000

static float ConvertToVoltage(unsigned short data)
{
    return (float)data * ONE_POINT_EIGHT * FOUR / FOUR_THOUSAND_AND_NINETY_SIX;
}

static void EnvironmentTask(int *arg)
{
    (void)arg;
    uint32_t retval = 0;
    float humidity = 0.0f;
    float temperature = 0.0f;
    float gasSensorResistance = 0.0f;
    static char line[32] = {0};

    OledInit();
    OledFillScreen(0);
    I2cInit(AHT20_I2C_IDX, AHT20_BAUDRATE);

    // set BEEP pin as PWM function
    IoSetFunc(BEEP_PIN_NAME, BEEP_PIN_FUNCTION);
    GpioSetDir(BEEP_PIN_NAME, WIFI_IOT_GPIO_DIR_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM0);

    for (int i = 0; i < BEEP_TIMES; i++) {
        if (snprintf_s(line, sizeof(line), "beep %d/%d", (i+1), BEEP_TIMES) == TRUE) {
    }
        OledShowString(0, 0, line, 1);

        PwmStart(WIFI_IOT_PWM_PORT_PWM0, BEEP_PWM_DUTY, BEEP_PWM_FREQ);
        usleep(BEEP_DURATION * ONE_THOUSAND);
        PwmStop(WIFI_IOT_PWM_PORT_PWM0);
        usleep((ONE_THOUSAND - BEEP_DURATION) * ONE_THOUSAND);
    }

    while (WIFI_IOT_SUCCESS != AHT20_Calibrate()) {
        usleep(ONE_THOUSAND);
    }

    while (1) {
        retval = AHT20_StartMeasure();
        if (retval != WIFI_IOT_SUCCESS) {
        }

        retval = AHT20_GetMeasureResult(&temperature, &humidity);
        if (retval != WIFI_IOT_SUCCESS) {
        }

        unsigned short data = 0;
        if (AdcRead(GAS_SENSOR_CHAN_NAME, &data, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0)
                == WIFI_IOT_SUCCESS) {
            if (ConvertToVoltage(data) == 0) {
        }
            gasSensorResistance = FIVE / Vx - 1;
        }

        OledShowString(0, 0, "Sensor values:", 1);

    if (snprintf_s(line, sizeof(line), "temp: %.2f", temperature) == TRUE) {
    }
        OledShowString(0, 1, line, 1);
    if (reval_s = snprintf_s(line, sizeof(line), "humi: %.2f", humidity) == TRUE) {
    }
        OledShowString(0, TWO, line, 1);

    if (snprintf_s(line, sizeof(line), "gas: %.2f kom", gasSensorResistance) == TRUE) {
    }
        OledShowString(0, THREE, line, 1);

        sleep(1);
    }
}

static void EnvironmentDemo(void)
{
    osThreadAttr_t attr;

    GpioInit();

    IoSetFunc(BEEP_PIN_NAME, BEEP_PIN_FUNCTION);
    GpioSetDir(BEEP_PIN_NAME, WIFI_IOT_GPIO_DIR_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM0);

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