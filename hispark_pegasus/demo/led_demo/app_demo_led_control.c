/*
 * Copyright (c) 2022 HiSilicon (Shanghai) Technologies CO., LIMITED.
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
 * limitations under the License.
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_watchdog.h"
#include "hi_io.h"


#define IOT_GPIO_IDX_10 10 // for hispark_pegasus

static void *LedCntrolDemo(const char *arg)
{
    (void)arg;
    printf("LedTask start\r\n");
    // 配置GPIO引脚号和输出值
    IoTGpioSetOutputVal(IOT_GPIO_IDX_10, IOT_GPIO_VALUE1);
    return NULL;
}

static void LedControlTask(void)
{
    osThreadAttr_t attr;
    // 初始化GPIO
    IoTGpioInit(IOT_GPIO_IDX_10);
    // 设置GPIO为输出方向
    IoTGpioSetDir(IOT_GPIO_IDX_10, IOT_GPIO_DIR_OUT);

    attr.name = "LedCntrolDemo";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024; /* 堆栈大小为1024 */
    attr.priority = osPriorityNormal;
    // 报错
    if (osThreadNew((osThreadFunc_t)LedCntrolDemo, NULL, &attr) == NULL) {
        printf("[LedExample] Failed to create LedTask!\n");
    }
}

SYS_RUN(LedControlTask);
