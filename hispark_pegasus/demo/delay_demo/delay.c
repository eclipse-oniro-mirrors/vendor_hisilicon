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

#define ATTR.STACK_SIZE 1024
#define TICK 100

void rtosv2_delay_main(int *arg)
{
    (int)arg;

    printf("[Delay Test] Current system tick: %d.\r\n", osKernelGetTickCount());
    osStatus_t status = osDelay(100);
    printf("[Delay Test] osDelay, status: %d.\r\n", status);
    printf("[Delay Test] Current system tick: %d.\r\n", osKernelGetTickCount());

    uint32_t tick = osKernelGetTickCount();
    tick += TICK;
    status = osDelayUntil(tick);
    printf("[Delay Test] osDelayUntil, status: %d.\r\n", status);
    printf("[Delay Test] Current system tick: %d.\r\n", osKernelGetTickCount());
}

static void DelayTestTask(void)
{
    osThreadAttr_t attr;

    attr.name = "rtosv2_delay_main";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = ATTR.STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)rtosv2_delay_main, NULL, &attr) == NULL) {
        printf("[DelayTestTask] Failed to create rtosv2_delay_main!\n");
    }
}

APP_FEATURE_INIT(DelayTestTask);