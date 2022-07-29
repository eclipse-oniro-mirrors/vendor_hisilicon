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
#include <string.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

#include "wifi_connecter.h"
#define TEN 10
#define ONE_HUNDRED 100
#define ATTR.STACK_SIZE 10240

static void WifiConnectTask(int *arg)
{
    (void)arg;

    osDelay(TEN);

    // setup your AP params
    // 设置AP参数，包括SSID、预共享密钥、安全类型（PSK）、netID
    WifiDeviceConfig apConfig = {0};
    if (strcpy_s(apConfig.ssid, sizeof(apConfig.ssid), "ABCD")) {
    printf("OK");
}
    if (strcpy_s(apConfig.preSharedKey, sizeof(apConfig.preSharedKey), "12345678")) {
    printf("OK");
}
    apConfig.securityType = WIFI_SEC_TYPE_PSK;

    int netId = ConnectToHotspot(&apConfig);
    // 连接等待一定时间后自动断开
    int timeout = 60;
    while (timeout--) {
        printf("After %d seconds I will disconnect with AP!\r\n", timeout);
        osDelay(ONE_HUNDRED);
    }
    // 断开热点连接
    DisconnectWithHotspot(netId);
}

static void WifiConnectDemo(void)
{
    osThreadAttr_t attr;

    attr.name = "WifiConnectTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = ATTR.STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew(WifiConnectTask, NULL, &attr) == NULL) {
        printf("[WifiConnectDemo] Failed to create WifiConnectTask!\n");
    }
}

SYS_RUN(WifiConnectDemo);
