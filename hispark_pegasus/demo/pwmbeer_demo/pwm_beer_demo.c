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
#define TWENTY 20
#define FORTY 40
#define ONE_THOUSAND 1000

static int g_beepState = 0;

static void *PWMBeerTask(const char *arg)
{
    (void)arg;

    printf("PWMBeerTask start!\r\n");
    
    while (1) {
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
    g_beepState = !g_beepState;
}

static void StartPWMBeerTask(void)
{
    osThreadAttr_t attr;

    GpioInit();

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_IO_FUNC_GPIO_8_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_IO_PULL_UP);
    GpioRegisterIsrFunc(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_INT_TYPE_EDGE, WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW,
        OnButtonPressed, NULL);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_DIR_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM0);

    WatchDogDisable();

    attr.name = "PWMBeerTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = ATTR.STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)PWMBeerTask, NULL, &attr) == NULL) {
        printf("[StartPWMBeerTask] Failed to create PWMBeerTask!\n");
    }
}

APP_FEATURE_INIT(StartPWMBeerTask);