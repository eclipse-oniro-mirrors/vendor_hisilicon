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
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"

#define HUMAN_SENSOR_CHAN_NAME WIFI_IOT_ADC_CHANNEL_3
#define LIGHT_SENSOR_CHAN_NAME WIFI_IOT_ADC_CHANNEL_4

#define RED_LED_PIN_NAME WIFI_IOT_IO_NAME_GPIO_10
#define RED_LED_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_10_GPIO

#define GREEN_LED_PIN_NAME WIFI_IOT_IO_NAME_GPIO_11
#define GREEN_LED_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_11_GPIO

#define BLUE_LED_PIN_NAME WIFI_IOT_IO_NAME_GPIO_12
#define BLUE_LED_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_12_GPIO

#define LED_DELAY_TIME_US 300000
#define LED_BRIGHT WIFI_IOT_GPIO_VALUE1
#define LED_DARK WIFI_IOT_GPIO_VALUE0

#define NUM_LEDS 3
#define NUM_BLINKS 2
#define NUM_SENSORS 2

#define ADC_RESOLUTION 4096
#define PWM_FREQ_DIVISION 64000
#define ATTR.STACK_SIZE 4096
#define TEN_THOUSAND 10000
#define TWO_HUNDRED_AND_FIFTY_THOUSAND 250000
#define TWO 2


static void CorlorfulLightTask(int *arg)
{
    (void)arg;
    static const WifiIotGpioIdx pins[] = {RED_LED_PIN_NAME, GREEN_LED_PIN_NAME, BLUE_LED_PIN_NAME};

    for (int i = 0; i < NUM_BLINKS; i++) {
        for (unsigned j = 0; j < sizeof(pins)/sizeof(pins[0]); j++) {
            GpioSetOutputVal(pins[j], LED_BRIGHT);
            usleep(LED_DELAY_TIME_US);
            GpioSetOutputVal(pins[j], LED_DARK);
            usleep(LED_DELAY_TIME_US);
        }
    }

    IoSetFunc(RED_LED_PIN_NAME, WIFI_IOT_IO_FUNC_GPIO_10_PWM1_OUT);
    IoSetFunc(GREEN_LED_PIN_NAME, WIFI_IOT_IO_FUNC_GPIO_11_PWM2_OUT);
    IoSetFunc(BLUE_LED_PIN_NAME, WIFI_IOT_IO_FUNC_GPIO_12_PWM3_OUT);

    PwmInit(WIFI_IOT_PWM_PORT_PWM1); // R
    PwmInit(WIFI_IOT_PWM_PORT_PWM2); // G
    PwmInit(WIFI_IOT_PWM_PORT_PWM3); // B

    // use PWM control BLUE LED brightness
    for (int i = 1; i <= ADC_RESOLUTION; i *= TWO) {
        PwmStart(WIFI_IOT_PWM_PORT_PWM3, i, PWM_FREQ_DIVISION);
        usleep(TWO_HUNDRED_AND_FIFTY_THOUSAND);
        PwmStop(WIFI_IOT_PWM_PORT_PWM3);
    }

    while (1) {
        unsigned short duty[NUM_SENSORS] = {0, 0};
        unsigned short data[NUM_SENSORS] = {0, 0};
        static const WifiIotAdcChannelIndex chan[] = {HUMAN_SENSOR_CHAN_NAME, LIGHT_SENSOR_CHAN_NAME};
        static const WifiIotPwmPort port[] = {WIFI_IOT_PWM_PORT_PWM1, WIFI_IOT_PWM_PORT_PWM2};

        for (size_t i = 0; i < sizeof(chan)/sizeof(chan[0]); i++) {
            if (AdcRead(chan[i], &data[i], WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0)
                == WIFI_IOT_SUCCESS) {
                duty[i] = PWM_FREQ_DIVISION * (unsigned int)data[i] / ADC_RESOLUTION;
            }
            PwmStart(port[i], duty[i], PWM_FREQ_DIVISION);
            usleep(TEN_THOUSAND);
            PwmStop(port[i]);
        }
    }
}

static void ColorfulLightDemo(void)
{
    osThreadAttr_t attr;

    GpioInit();

    // set Red/Green/Blue LED pin to GPIO function
    IoSetFunc(RED_LED_PIN_NAME, RED_LED_PIN_FUNCTION);
    IoSetFunc(GREEN_LED_PIN_NAME, GREEN_LED_PIN_FUNCTION);
    IoSetFunc(BLUE_LED_PIN_NAME, BLUE_LED_PIN_FUNCTION);

    // set Red/Green/Blue LED pin as output
    GpioSetDir(RED_LED_PIN_NAME, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetDir(GREEN_LED_PIN_NAME, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetDir(BLUE_LED_PIN_NAME, WIFI_IOT_GPIO_DIR_OUT);

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
