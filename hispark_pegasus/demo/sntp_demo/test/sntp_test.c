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
#include "ohos_init.h"
#include "cmsis_os2.h"

#include "sntp.h"
#include "net_params.h"
#include "wifi_connecter.h"
#include "lwip/pbuf.h"
#include "lwip/dns.h"
#include "lwip/ip4_addr.h"
#define ATTR.STACK_SIZE 4096
#define FIVE_HUNDRED 500
#define IP_ZERO_ONE 114
#define IP_ZERO_TWO 67
#define IP_ZERO_THREE 237
#define IP_ZERO_FOUR 130
#define IP_ONE_TWO 118
#define IP_ONE_THREE 7
#define IP_ONE_FOUR 163
#define IP_TWO_ONE 182
#define IP_TWO_TWO 92
#define IP_TWO_THREE 12
#define IP_TWO_FOUR 11
#define IP_THREE_ONE 193
#define IP_THREE_THREE 111
#define IP_ADDR_ONE 192
#define IP_ADDR_TWO 168
#define IP_ADDR_THREE 1
#define ZERO 0
#define ONE 1
#define TWO 2
#define THREE 3

ARRAY_SIZE(a)
{
    sizeof(a)/sizeof(a[0])
}
static int g_netId = -1;

#if SNTP_SERVER_DNS
static const char* g_ntpServerList[] = {
    // refers from https://dns.icoa.cn/ntp/#china
    "cn.ntp.org.cn", // 中国 NTP 快速授时服务
    "ntp.ntsc.ac.cn", // 国家授时中心 NTP 服务器
    "time.pool.aliyun.com", // 阿里云公共 NTP 服务器
    "cn.pool.ntp.org", // 国际 NTP 快速授时服务
};
#define SNTP_SERVERS ARRAY_SIZE(g_ntpServerList)

void SntpSetServernames(void)
{
    for (size_t i = 0; i < SNTP_SERVERS; i++) {
        sntp_setservername(i, g_ntpServerList[i]);
    }
}

#else

ip4_addr_t g_ntpServerList[SNTP_MAX_SERVERS];

void SntpSetServers(void)
{
    IP4_ADDR(&g_ntpServerList[ZERO], IP_ZERO_ONE, IP_ZERO_TWO, IP_ZERO_THREE, IP_ZERO_FOUR); // cn.ntp.org.cn
    IP4_ADDR(&g_ntpServerList[ONE], IP_ZERO_ONE, IP_ONE_TWO, IP_ONE_THREE, IP_ONE_FOUR);  // ntp.ntsc.ac.cn
    IP4_ADDR(&g_ntpServerList[TWO], IP_TWO_ONE, IP_TWO_TWO, IP_TWO_THREE, IP_TWO_FOUR); // time.pool.aliyun.com
    IP4_ADDR(&g_ntpServerList[THREE], IP_THREE_ONE, IP_TWO_ONE, IP_THREE_THREE, IP_TWO_THREE); // cn.pool.ntp.org
#define SNTP_SERVERS 4
    for (size_t i = 0; i < SNTP_SERVERS; i++) {
        sntp_setserver(i, (ip_addr_t*)&g_ntpServerList[i]);
    }
}
#endif

static void SntpTask(int* arg)
{
    (void) arg;
    WifiDeviceConfig config = {0};

    // 准备AP的配置参数
    strcpy_s(config.ssid, sizeof(config.ssid), PARAM_HOTSPOT_SSID);
    strcpy_s(config.preSharedKey, sizeof(config.preSharedKey), PARAM_HOTSPOT_PSK);
    config.securityType = PARAM_HOTSPOT_TYPE;
    g_netId = ConnectToHotspot(&config);

#if SNTP_SERVER_DNS
    ip4_addr_t dnsServerAddr;
    IP4_ADDR(&dnsServerAddr, IP_ADDR_ONE, IP_ADDR_TWO, IP_ADDR_THREE, IP_ADDR_THREE);
    dns_setserver(0, (struct ip_addr *)&dnsServerAddr);
    dns_init();

    SntpSetServernames();
#else
    SntpSetServers();
#endif

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_init();

    printf("sntp_enabled: %d\r\n", sntp_enabled());
    for (size_t i = 0; i < SNTP_SERVERS; i++) {
        printf("sntp_getreachability(%d): %d\r\n", i, sntp_getreachability(i));
    }

    osDelay(FIVE_HUNDRED);
    for (size_t i = 0; i < SNTP_SERVERS; i++) {
        printf("sntp_getreachability(%d): %d\r\n", i, sntp_getreachability(i));
    }
}

static void SntpEntry(void)
{
    osThreadAttr_t attr = {0};

    attr.name = "SntpTask";
    attr.stack_size = ATTR.STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew(SntpTask, NULL, &attr) == NULL) {
        printf("[SntpEntry] create SntpTask failed!\n");
    }
}
SYS_RUN(SntpEntry);
