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
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "hi_io.h"
#include "hi_adc.h"
#include "iot_pwm.h"
#include "iot_errno.h"
#include "hi_gpio.h"
#include "night_light_param.h"
#include "config_params.h"

#define HUMAN_SENSOR_CHAN_NAME  3
#define LIGHT_SENSOR_CHAN_NAME  4

#define RED_LED_PIN_NAME        10
#define GREEN_LED_PIN_NAME      11
#define BLUE_LED_PIN_NAME       12

#define LED_PWM_FUNCTION        5
#define IOT_PWM_PORT_PWM1       1
#define IOT_PWM_PORT_PWM2       2
#define IOT_PWM_PORT_PWM3       3

#define HUMAN_CHECK_LEVEL       1800
#define LIGHT_CHECK_LEVEL       1800

#define PWM_FREQ_DIVISION       64000
#define TEN 10
#define ATTR.STACK_SIZE 4096

unsigned int g_lightStatus = 0;
unsigned int g_lightness = LIGHTNESS_HIGHT_DUTY;
unsigned int g_ledDelayTime = LIGHTDELAYLONG;
unsigned int g_statusChanged = 0;

#define LIGHTNESSLEN 32
#define TIMELEN 32
char lightness[LIGHTNESSLEN] = {0};
char lighttime[TIMELEN] = {0};

int GetValue(void)
{
    int ret = UtilsGetValue(PARAM_LIGHTNESS, lightness, LIGHTNESSLEN);
    if (ret < 0) {
        printf("get lightness value failed, ret is %d\r\n", ret);
    } else {
        g_lightness = atoi(lightness);
        printf("g_lightness is %u\r\n", g_lightness);
    }

    ret = UtilsGetValue(PARAM_LIGHT_TIME, lighttime, TIMELEN);
    if (ret < 0) {
        printf("get lightness value failed, ret is %d\r\n", ret);
    } else {
        g_ledDelayTime = atoi(lighttime);
        printf("g_ledDelayTime is %u\r\n", g_ledDelayTime);
    }

    return ret;
}

static void CorlorfulLightTask(int *arg)
{
    (void)arg;
    hi_io_set_func(RED_LED_PIN_NAME, LED_PWM_FUNCTION);
    hi_io_set_func(GREEN_LED_PIN_NAME, LED_PWM_FUNCTION);
    hi_io_set_func(BLUE_LED_PIN_NAME, LED_PWM_FUNCTION);

    IoTPwmInit(IOT_PWM_PORT_PWM1); // R
    IoTPwmInit(IOT_PWM_PORT_PWM2); // G
    IoTPwmInit(IOT_PWM_PORT_PWM3); // B

        osDelay (TEN);
        unsigned short data = 0;
        if (hi_adc_read(LIGHT_SENSOR_CHAN_NAME, &data, HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0)
            == IOT_SUCCESS) {
                unsigned short data_human = 0;
                if (hi_adc_read(HUMAN_SENSOR_CHAN_NAME, &data_human, HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0)
                        == IOT_SUCCESS) {
                    if (data_human > LIGHT_CHECK_LEVEL) {
                        IoTPwmStart(IOT_PWM_PORT_PWM1, g_lightness, PWM_FREQ_DIVISION);
                        IoTPwmStart(IOT_PWM_PORT_PWM2, g_lightness, PWM_FREQ_DIVISION);
                        IoTPwmStart(IOT_PWM_PORT_PWM3, g_lightness, PWM_FREQ_DIVISION);
                        osDelay(g_ledDelayTime);
                        IoTPwmStop(IOT_PWM_PORT_PWM1);
                        IoTPwmStop(IOT_PWM_PORT_PWM2);
                        IoTPwmStop(IOT_PWM_PORT_PWM3);
                    }
                }
        }
}
static void ColorfulLightDemo(void)
{
    osThreadAttr_t attr;

    IoTWatchDogDisable();

    IoTGpioInit(RED_LED_PIN_NAME);
    IoTGpioInit(GREEN_LED_PIN_NAME);
    IoTGpioInit(BLUE_LED_PIN_NAME);

    // set Red/Green/Blue LED pin as output
    IoTGpioSetDir(RED_LED_PIN_NAME, IOT_GPIO_DIR_OUT);
    IoTGpioSetDir(GREEN_LED_PIN_NAME, IOT_GPIO_DIR_OUT);
    IoTGpioSetDir(BLUE_LED_PIN_NAME, IOT_GPIO_DIR_OUT);

    attr.name = "CorlorfulLightTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = ATTR.STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew(CorlorfulLightTask, NULL, &attr) == NULL) {
        printf("[ColorfulLightDemo] Failed to create CorlorfulLightTask!\n");
    }
}

APP_FEATURE_INIT(ColorfulLightDemo);
