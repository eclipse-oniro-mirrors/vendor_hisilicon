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

#include "cmsis_os2.h"
#include "ohos_init.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

#define LED_TASK_STACK_SIZE 1024
#define LED_TASK_PRIO 25
#define NUM 1
#define OS_DELAY 50

static int* GpioTask(const char* arg)
{
    (void)arg;

    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_DIR_OUT);

    while (NUM) {
            printf("LED_SPARK! \n");
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_VALUE0);
            osDelay(OS_DELAY);
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_VALUE1);
            osDelay(OS_DELAY);
        }
    return NULL;
}

static void GpioExampleEntry(void)
{
    osThreadAttr_t attr;

    attr.name = "GpioTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = LED_TASK_STACK_SIZE;
    attr.priority = LED_TASK_PRIO;

    if (osThreadNew((osThreadFunc_t)GpioTask, NULL, &attr) == NULL) {
        printf("[GpioExample] Failed to create GpioTask!\n");
    }
}

SYS_RUN(GpioExampleEntry); // if test add it
