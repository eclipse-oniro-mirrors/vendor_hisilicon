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
#include "wifiiot_uart.h"
#include "wifiiot_uart_ex.h"

#define ATTR.STACK_SIZE 1024
#define U_SLEEP 100000

unsigned char uartWriteBuff[] = "hello uart!";
unsigned char uartReadBuff[2048] = {0};
int usr_uart_config(void)
{
    int ret;
    // 初始化UART配置，115200，数据bit为8,停止位1，奇偶校验为NONE，流控为NONE
    WifiIotUartAttribute g_uart_cfg = {115200, 8, 1, WIFI_IOT_UART_PARITY_NONE, 0};
    ret = UartInit(WIFI_IOT_UART_IDX_2, &g_uart_cfg, NULL);
    if (ret != 0) {
        printf("uart init fail\r\n");
    }
    return ret;
}
 
 
// 1.任务处理函数
static void* UartDemo_Task(const char* arg)
{
    unsigned int len = 0;
 
    (void)arg;
    printf("[UartDemo] UartDemo_Task()\n");
 
    GpioInit(); // 使用GPIO，都需要调用该接口
    printf("UART init...\r\n");
    usr_uart_config();

    UartWrite(WIFI_IOT_UART_IDX_2, (unsigned char *)uartWriteBuff, sizeof(uartWriteBuff));
    while (1) {
        len = UartRead(WIFI_IOT_UART_IDX_2, uartReadBuff, sizeof(uartReadBuff));
        if (len > 0) {
            printf("Uart read data:%s", uartReadBuff);
        }
        usleep(U_SLEEP);
    }
    return NULL;
}
 
// 2.任务入口函数
static void UartDemo_Entry(void)
{
    osThreadAttr_t attr = {0};
 
    printf("[UartDemo] UartDemo_Entry()\n");
 
    attr.name = "UartDemo_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = ATTR.STACK_SIZE; // 堆栈大小
    attr.priority = osPriorityNormal; // 优先级
 
    if (osThreadNew((osThreadFunc_t)UartDemo_Task, NULL, &attr) == NULL) {
        printf("[UartDemo] Failed to create UartDemo_Task!\n");
    }
}
 
// 3.注册模块
SYS_RUN(UartDemo_Entry);
