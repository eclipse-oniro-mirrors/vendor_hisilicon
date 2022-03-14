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
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#include "net_demo.h"
#include "net_params.h"
#include "wifi_connecter.h"

static void NetDemoTask(void)
{
    int netId = ConnectToHotspot();

    int timeout = 10; /* timeout 10ms */
    while (timeout--) {
        printf("After %d seconds, I will start lwip test!\r\n", timeout);
        osDelay(100); /* 延时100ms */
    }

    NetDemoTest(PARAM_SERVER_PORT, PARAM_SERVER_ADDR);

    printf("disconnect to AP ...\r\n");
    DisconnectWithHotspot(netId);
    printf("disconnect to AP done!\r\n");
}

static void NetDemoEntry(void)
{
    osThreadAttr_t attr;

    attr.name = "NetDemoTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240; /* 堆栈大小为10240 */
    attr.priority = osPriorityNormal;

    if (osThreadNew(NetDemoTask, NULL, &attr) == NULL) {
        printf("[NetDemoEntry] Falied to create NetDemoTask!\n");
    }
}

SYS_RUN(NetDemoEntry);