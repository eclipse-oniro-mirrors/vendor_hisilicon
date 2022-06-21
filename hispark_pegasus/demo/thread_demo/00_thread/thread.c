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

#define STACK_SIZE 1024
#define OS_DELAY 100
#define OS_DELAYONE 20

osThreadId_t newThread(char *name, osThreadFunc_t func, int*arg)
{
    osThreadAttr_t attr = {
        name, 0, NULL, 0, NULL, 1024*2, osPriorityNormal, 0, 0
    };
    osThreadId_t tid = osThreadNew(func, arg, &attr);
    if (tid == NULL) {
        printf("[Thread Test] osThreadNew(%s) failed.\r\n", name);
    } else {
        printf("[Thread Test] osThreadNew(%s) success, thread id: %d.\r\n", name, tid);
    }
    return tid;
}

void threadTest(int *arg)
{
    static int count = 0;
    printf("%s\r\n", (char *)arg);
    osThreadId_t tid = osThreadGetId();
    printf("[Thread Test] threadTest osThreadGetId, thread id:%p\r\n", tid);
    while (1) {
        count++;
        printf("[Thread Test] threadTest, count: %d.\r\n", count);
        osDelay(OS_DELAYONE);
    }
}

void rtosv2_thread_main(int *arg)
{
    (void)arg;
    osThreadId_t tid = newThread("test_thread", threadTest, "This is a test thread.");

    const char *t_name = osThreadGetName(tid);
    printf("[Thread Test] osThreadGetName, thread name: %s.\r\n", t_name);

    osThreadState_t state = osThreadGetState(tid);
    printf("[Thread Test] osThreadGetState, state :%d.\r\n", state);

    osStatus_t status = osThreadSetPriority(tid, osPriorityNormal4);
    printf("[Thread Test] osThreadSetPriority, status: %d.\r\n", status);

    osPriority_t pri = osThreadGetPriority(tid);
    printf("[Thread Test] osThreadGetPriority, priority: %d.\r\n", pri);

    status = osThreadSuspend(tid);
    printf("[Thread Test] osThreadSuspend, status: %d.\r\n", status);

    status = osThreadResume(tid);
    printf("[Thread Test] osThreadResume, status: %d.\r\n", status);

    uint32_t stacksize = osThreadGetStackSize(tid);
    printf("[Thread Test] osThreadGetStackSize, stacksize: %u.\r\n", stacksize);

    uint32_t stackspace = osThreadGetStackSpace(tid);
    printf("[Thread Test] osThreadGetStackSpace, stackspace: %u.\r\n", stackspace);

    uint32_t t_count = osThreadGetCount();
    printf("[Thread Test] osThreadGetCount, count: %u.\r\n", t_count);

    osDelay(OS_DELAY);
    status = osThreadTerminate(tid);
    printf("[Thread Test] osThreadTerminate, status: %d.\r\n", status);
}

static void ThreadTestTask(void)
{
    osThreadAttr_t attr;
    attr.name = "rtosv2_thread_main";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)rtosv2_thread_main, NULL, &attr) == NULL) {
        printf("[ThreadTestTask] Failed to create rtosv2_thread_main!\n");
    }
}

APP_FEATURE_INIT(ThreadTestTask);