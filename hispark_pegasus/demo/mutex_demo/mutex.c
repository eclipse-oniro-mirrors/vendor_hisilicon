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
#define OS_DELAY 5
#define OS_DELAY_F 13
#define OS_DELAY_S 17
#define NUM 100
#define NUMS 2

static int g_test_value = 0;

void number_thread(int *arg)
{
    osMutexId_t *mid = (osMutexId_t *)arg;
    while (1) {
        if (osMutexAcquire(*mid, NUM) == osOK) {
            g_test_value++;
            if (g_test_value % NUMS == 0) {
                printf("[Mutex Test]%s gets an even value %d.\r\n", osThreadGetName(osThreadGetId()), g_test_value);
            } else {
                printf("[Mutex Test]%s gets an odd value %d.\r\n",  osThreadGetName(osThreadGetId()), g_test_value);
            }
            osMutexRelease(*mid);
            osDelay(OS_DELAY);
        }
    }
}

osThreadId_t newThread(char *name, osThreadFunc_t func, int *arg)
{
    osThreadAttr_t attr = {
        name, 0, NULL, 0, NULL, 1024*2, osPriorityNormal, 0, 0
    };
    osThreadId_t tid = osThreadNew(func, arg, &attr);
    if (tid == NULL) {
        printf("[Mutex Test]osThreadNew(%s) failed.\r\n", name);
    } else {
        printf("[Mutex Test]osThreadNew(%s) success, thread id: %d.\r\n", name, tid);
    }
    return tid;
}

void rtosv2_mutex_main(int *arg)
{
    (int)arg;
    osMutexAttr_t attr = {0};

    osMutexId_t mid = osMutexNew(&attr);
    if (mid == NULL) {
        printf("[Mutex Test]osMutexNew, create mutex failed.\r\n");
    } else {
        printf("[Mutex Test]osMutexNew, create mutex success.\r\n");
    }

    osThreadId_t tid1 = newThread("Thread_1", number_thread, &mid);
    osThreadId_t tid2 = newThread("Thread_2", number_thread, &mid);
    osThreadId_t tid3 = newThread("Thread_3", number_thread, &mid);

    osDelay(OS_DELAY_F);
    osThreadId_t tid = osMutexGetOwner(mid);
    printf("[Mutex Test]osMutexGetOwner, thread id: %p, thread name: %s.\r\n", tid, osThreadGetName(tid));
    osDelay(OS_DELAY_S);

    osThreadTerminate(tid1);
    osThreadTerminate(tid2);
    osThreadTerminate(tid3);
    osMutexDelete(mid);
}

static void MutexTestTask(void)
{
    osThreadAttr_t attr;

    attr.name = "rtosv2_mutex_main";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = ATTR.STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)rtosv2_mutex_main, NULL, &attr) == NULL) {
        printf("[MutexTestTask] Failed to create rtosv2_mutex_main!\n");
    }
}

APP_FEATURE_INIT(MutexTestTask);