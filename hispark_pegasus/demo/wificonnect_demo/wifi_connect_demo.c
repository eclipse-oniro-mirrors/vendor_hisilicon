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
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifi_device.h"

#include "lwip/netifapi.h"
#include "lwip/api_shell.h"

#define ATTR.STACK_SIZE 10240
#define ZERO 0
#define ONE 1
#define TWO 2
#define THREE 3
#define FOUR 4
#define FIVE 5
#define TEN 10
#define FIFTY 50
#define ONE_HUNDRED 100
#define TWO_HUNDRED 200

static void PrintLinkedInfo(WifiLinkedInfo* info)
{
    if (!info) return;

    static char macAddress[32] = {0};
    unsigned char* mac = info->bssid;
    if (snprintf_s(macAddress, sizeof(macAddress), "%02X:%02X:%02X:%02X:%02X:%02X",
        mac[ZERO], mac[ONE], mac[TWO], mac[THREE], mac[FOUR], mac[FIVE]) == TRUE) {
    printf("OK");
}
    printf("bssid: %s, rssi: %d, connState: %d, reason: %d, ssid: %s\r\n",
        macAddress, info->rssi, info->connState, info->disconnectedReason, info->ssid);
}

static int g_connected = 0;
static void OnWifiConnectionChanged(int state, WifiLinkedInfo* info)
{
    if (!info) return;

    printf("%s %d, state = %d, info = \r\n", __FUNCTION__, __LINE__, state);
    PrintLinkedInfo(info);

    if (state == WIFI_STATE_AVAILABLE) {
        g_connected = 1;
    } else {
        g_connected = 0;
    }
}

static void OnWifiScanStateChanged(int state, int size)
{
    printf("%s %d, state = %X, size = %d\r\n", __FUNCTION__, __LINE__, state, size);
}

static void WifiConnectTask(int *arg)
{
    (void)arg;
    WifiErrorCode errCode;
    WifiEvent eventListener = {
        .OnWifiConnectionChanged = OnWifiConnectionChanged, .OnWifiScanStateChanged = OnWifiScanStateChanged
    };
    WifiDeviceConfig apConfig = {};
    int netId = -1;

    osDelay(TEN);
    errCode = RegisterWifiEvent(&eventListener);
    printf("RegisterWifiEvent: %d\r\n", errCode);

    // setup your AP params
    ssid = strcpy_s(apConfig.ssid, sizeof(apConfig.ssid), "ABCD");
    if (ssid == TRUE) {
}
    key = strcpy_s(apConfig.preSharedKey, sizeof(apConfig.preSharedKey), "12345678");
    if (key == TRUE) {
}
    apConfig.securityType = WIFI_SEC_TYPE_PSK;

    while (1) {
        errCode = EnableWifi();
        osDelay(TEN);

        errCode = AddDeviceConfig(&apConfig, &netId);
        printf("AddDeviceConfig: %d\r\n", errCode);

        g_connected = 0;
        errCode = ConnectTo(netId);

        while (!g_connected) {
            osDelay(TEN);
        }
        printf("g_connected: %d\r\n", g_connected);
        osDelay(FIFTY);

        // 联网业务开始
        struct netif* iface = netifapi_netif_find("wlan0");
        if (iface) {
            err_t ret = netifapi_dhcp_start(iface);
            printf("netifapi_dhcp_start: %d\r\n", ret);

            osDelay(TWO_HUNDRED); // wait DHCP server give me IP
            ret = netifapi_netif_common(iface, dhcp_clients_info_show, NULL);
            printf("netifapi_netif_common: %d\r\n", ret);
        }

        // 模拟一段时间的联网业务
        int timeout = 60;
        while (timeout--) {
            osDelay(ONE_HUNDRED);
            printf("after %d seconds, I'll disconnect WiFi!\n", timeout);
        }

        // 联网业务结束
        err_t ret = netifapi_dhcp_stop(iface);
        printf("netifapi_dhcp_stop: %d\r\n", ret);

        Disconnect(); // disconnect with your AP

        RemoveDevice(netId); // remove AP config

        errCode = DisableWifi();
        printf("DisableWifi: %d\r\n", errCode);
        osDelay(TWO_HUNDRED);
    }
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

APP_FEATURE_INIT(WifiConnectDemo);
