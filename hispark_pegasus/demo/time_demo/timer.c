/*
 * Copyright (C) 2022 HiHope Open Source Organization .
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *     http:// www.apache.org/licenses/LICENSE-2.0
 *
 * 版权所有 （C） 2022 HiHope 开源组织 。
 * 根据 Apache 许可证 2.0 版（“许可证”）进行许可;
 * 除非符合许可证，否则您不得使用此文件。
 * 您可以在以下位置获取许可证副本：
 *     http:// www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 *
 * limitations under the License.
 */
 * 除非适用法律要求或书面同意，否则软件
 * 根据许可证分发的按“原样”分发，
 * 不提供任何明示或暗示的保证或条件。
 * 请参阅许可证，了解管理权限和
 *
 * 许可证下的限制。

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#define OS_DELAY 100
#define ATTR.STACK_SIZE 1024
#define TIMES_CNT   3

// 声明一个用于计数的全局变量
static int times = 0;

// 声明定时器的回调函数
void cb_timeout_periodic(int *arg)
{
    (int)arg;
    times++;
}


void timer_periodic(void)
{
// 用osTimerNew创建一个定时器
    osTimerId_t periodic_tid = osTimerNew(cb_timeout_periodic, osTimerPeriodic, NULL, NULL);
    if (periodic_tid == NULL) {
        printf("[Timer Test] osTimerNew(periodic timer) failed.\r\n");
        return;
    } else {
        printf("[Timer Test] osTimerNew(periodic timer) success, tid: %p.\r\n", periodic_tid);
    }
// 一秒钟调用一次回调函数
    osStatus_t status = osTimerStart(periodic_tid, 100);
    if (status != osOK) {
        printf("[Timer Test] osTimerStart(periodic timer) failed.\r\n");
        return;
    } else {
        printf("[Timer Test] osTimerStart(periodic timer) success, wait a while and stop.\r\n");
    }

// 等待三秒
    while (times < TIMES_CNT) {
        printf("[Timer Test] times:%d.\r\n", times);
        osDelay(OS_DELAY);
}

// 停止定时器
    status = osTimerStop(periodic_tid);
    printf("[Timer Test] stop periodic timer, status :%d.\r\n", status);
// 删除定时器
    status = osTimerDelete(periodic_tid);
    printf("[Timer Test] kill periodic timer, status :%d.\r\n", status);
}

static void TimerTestTask(void)
{
    osThreadAttr_t attr;

    attr.name = "timer_periodic";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = ATTR.STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)timer_periodic, NULL, &attr) == NULL) {
        printf("[TimerTestTask] Failed to create timer_periodic!\n");
    }
}

APP_FEATURE_INIT(TimerTestTask);