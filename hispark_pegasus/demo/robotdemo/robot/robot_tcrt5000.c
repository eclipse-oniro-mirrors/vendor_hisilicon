/*
 * Copyright (C) 2022 HiHope Open Source Organization .
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 *
 * limitations under the License.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "hi_io.h"
#include "hi_time.h"


#define GPIO11 11
#define GPIO12 12
#define TASK_STAK_SIZE    (1024*10)
void get_tcrt5000_value (void)
{
    IotGpioValue id_status;
    IoTGpioGetInputVal(GPIO11, &id_status);
    if (id_status == IOT_GPIO_VALUE0) {
        printf("left black\r\n");
    } else {
        printf("left white\r\n");
    }
    IoTGpioGetInputVal(GPIO12, &id_status);
    if (id_status == IOT_GPIO_VALUE0) {
        printf("right black\r\n");
    } else {
        printf("right white\r\n");
    }
}

void RobotTask(void* parame)
{
    (void)parame;
    printf("start test tcrt5000\r\n");
    unsigned int time = 2000;
    while (1) {
        hi_sleep(time);
        get_tcrt5000_value();
    }
}


static void RobotDemo(void)
{
    osThreadAttr_t attr;

    attr.name = "RobotTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TASK_STAK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew(RobotTask, NULL, &attr) == NULL) {
        printf("[RobotDemo] Falied to create RobotTask!\n");
    }
}

APP_FEATURE_INIT(RobotDemo);
