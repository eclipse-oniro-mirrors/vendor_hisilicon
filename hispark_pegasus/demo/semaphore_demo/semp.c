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

#define OS_DELAY 4
#define OS_DELAY_F 50
#define OS_DELAY_S 3
#define NUM 1
#define ATTR.STACK_SIZE 1024
#define BUFFER_SIZE 5U
static int product_number = 0;
osSemaphoreId_t empty_id;
osSemaphoreId_t filled_id;

void producer_thread(int *arg)
{
    (int)arg;
    empty_id = osSemaphoreNew(BUFFER_SIZE, BUFFER_SIZE, NULL);
    filled_id = osSemaphoreNew(BUFFER_SIZE, 0U, NULL);
    while (1) {
        osSemaphoreAcquire(empty_id, osWaitForever);
        product_number++;
        printf("[Semp Test]%s produces a product, now product number: %d.\r\n",
            osThreadGetName(osThreadGetId()), product_number);
        osDelay(OS_DELAY);
        osSemaphoreRelease(filled_id);
    }
}

void consumer_thread(int *arg)
{
    (int)arg;
    while (NUM) {
        osSemaphoreAcquire(filled_id, osWaitForever);
        product_number--;
        printf("[Semp Test]%s consumes a product, now product number: %d.\r\n",
            osThreadGetName(osThreadGetId()), product_number);
        osDelay(OS_DELAY_S);
        osSemaphoreRelease(empty_id);
    }
}

osThreadId_t newThread(char *name, osThreadFunc_t func, int *arg)
{
    osThreadAttr_t attr = {
        name, 0, NULL, 0, NULL, 1024*2, osPriorityNormal, 0, 0
    };
    osThreadId_t tid = osThreadNew(func, arg, &attr);
    if (tid == NULL) {
        printf("[Semp Test]osThreadNew(%s) failed.\r\n", name);
    } else {
        printf("[Semp Test]osThreadNew(%s) success, thread id: %d.\r\n", name, tid);
    }
    return tid;
}

void rtosv2_semp_main(int *arg)
{
    (int)arg;
    empty_id = osSemaphoreNew(BUFFER_SIZE, BUFFER_SIZE, NULL);
    filled_id = osSemaphoreNew(BUFFER_SIZE, 0U, NULL);
 
    osThreadId_t ptid1 = newThread("producer1", producer_thread, NULL);
    osThreadId_t ptid2 = newThread("producer2", producer_thread, NULL);
    osThreadId_t ptid3 = newThread("producer3", producer_thread, NULL);
    osThreadId_t ctid1 = newThread("consumer1", consumer_thread, NULL);
    osThreadId_t ctid2 = newThread("consumer2", consumer_thread, NULL);

    osDelay(OS_DELAY_F);

    osThreadTerminate(ptid1);
    osThreadTerminate(ptid2);
    osThreadTerminate(ptid3);
    osThreadTerminate(ctid1);
    osThreadTerminate(ctid2);

    osSemaphoreDelete(empty_id);
    osSemaphoreDelete(filled_id);
}

static void SempTestTask(void)
{
    osThreadAttr_t attr;

    attr.name = "rtosv2_semp_main";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = ATTR.STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)rtosv2_semp_main, NULL, &attr) == NULL) {
        printf("[SempTestTask] Failed to create rtosv2_semp_main!\n");
    }
}

APP_FEATURE_INIT(SempTestTask);