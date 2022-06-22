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
#include "wifiiot_watchdog.h"
#include "wifiiot_pwm.h"

#define ATTR.STACK_SIZE 1024
#define FOUR 4
#define THREE 3
#define TWENTY 20
#define FORTY 40
#define ONE_HUNDRED 100
#define TWO_HUNDRED 200
#define ONE_THOUSAND 1000

static int g_ledStates[3] = {0, 0, 0};
static int g_currentBright = 0;
static int g_beepState = 0;

static void *TrafficLightTask(const char *arg)
{
    (void)arg;

    printf("TrafficLightTask start!\r\n");
    WifiIotGpioIdx pins[] = {WIFI_IOT_GPIO_IDX_10, WIFI_IOT_GPIO_IDX_11, WIFI_IOT_GPIO_IDX_12};
    for (int i = 0; i < FOUR; i++) {
        for (unsigned int j = 0; j < THREE; j++) {
            GpioSetOutputVal(pins[j], WIFI_IOT_GPIO_VALUE1);
            usleep(TWO_HUNDRED*ONE_THOUSAND);

            GpioSetOutputVal(pins[j], WIFI_IOT_GPIO_VALUE0);
            usleep(ONE_HUNDRED*ONE_THOUSAND);
        }
    }

    while (1) {
        for (unsigned int j = 0; j < THREE; j++) {
            GpioSetOutputVal(pins[j], g_ledStates[j]);
        }
        if (g_beepState) {
            PwmStart(WIFI_IOT_PWM_PORT_PWM0, TWENTY*ONE_THOUSAND, FORTY*ONE_THOUSAND);
        } else {
            PwmStop(WIFI_IOT_PWM_PORT_PWM0);
        }
    }

    return NULL;
}

static void OnButtonPressed(char *arg)
{
    (void) arg;
    for (int i = 0; i < THREE; i++) {
        if (i == g_currentBright) {
            g_ledStates[i] = 1;
        } else {
            g_ledStates[i] = 0;
        }
    }
    g_currentBright++;
    if (g_currentBright == THREE) {
        g_currentBright = 0;
}

    g_beepState = !g_beepState;
}

static void StartTrafficLightTask(void)
{
    osThreadAttr_t attr;

    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_IO_FUNC_GPIO_10_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_GPIO_DIR_OUT);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_FUNC_GPIO_11_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_GPIO_DIR_OUT);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_IO_FUNC_GPIO_12_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_GPIO_DIR_OUT);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_IO_FUNC_GPIO_8_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_IO_PULL_UP);
    GpioRegisterIsrFunc(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_INT_TYPE_EDGE, WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW,
        OnButtonPressed, NULL);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_DIR_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM0);

    WatchDogDisable();

    attr.name = "TrafficLightTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = ATTR.STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)TrafficLightTask, NULL, &attr) == NULL) {
        printf("[LedExample] Failed to create TrafficLightTask!\n");
    }
}

APP_FEATURE_INIT(StartTrafficLightTask);