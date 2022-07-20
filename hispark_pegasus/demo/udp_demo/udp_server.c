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

#include "lwip/netifapi.h"

#include <hi_io.h>
#include <hi_gpio.h>
#include <hi_task.h>
#include <hi_watchdog.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "udp_config.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#define INVAILD_SOCKET          (-1)
#define FREE_CPU_TIME_20MS      (20)
#define INVALID_VALUE           "202.202.202.202"

#define NATIVE_IP_ADDRESS       "XXX.XXX.XX.XXX" // 用户查找本地IP后需要进行修改
#define WECHAT_MSG_LIGHT_ON     "_light_on"
#define WECHAT_MSG_LIGHT_OFF    "_light_off"
#define DEVICE_MSG_LIGHT_ON     "device_light_on"
#define DEVICE_MSG_LIGHT_OFF    "device_light_off"
#define WECHAT_MSG_UNLOAD_PAGE  "UnoladPage"
#define RECV_DATA_FLAG_OTHER    (2)
#define HOST_PORT               (5566)
#define DEVICE_PORT             (6655)

#define UDP_RECV_LEN (255)

typedef void (*FnMsgCallBack)(hi_gpio_value val);

typedef struct FunctionCallback {
    hi_bool stop;
    hi_u32 conLost;
    hi_u32 queueID;
    hi_u32 iotTaskID;
    FnMsgCallBack msgCallBack;
}FunctionCallback;
FunctionCallback g_gfnCallback;

void DeviceConfigInit(hi_gpio_value val)
{
    hi_io_set_func(HI_IO_NAME_GPIO_9, HI_IO_FUNC_GPIO_9_GPIO);
    hi_gpio_set_dir(HI_GPIO_IDX_9, HI_GPIO_DIR_OUT);
    hi_gpio_set_ouput_val(HI_GPIO_IDX_9, val);
}

int  DeviceMsgCallback(FnMsgCallBack msgCallBack)
{
    g_gfnCallback.msgCallBack = msgCallBack;
    return 0;
}

void WeChatControlDeviceMsg(hi_gpio_value val)
{
    DeviceConfigInit(val);
}

int UdpTransportInit(struct sockaddr_in serAddr, struct sockaddr_in remoteAddr)
{
    int sServer = socket(AF_INET, SOCK_DGRAM, 0);
    if (sServer == INVAILD_SOCKET) {
        printf("create server socket failed\r\n");
        close(sServer);
    }
    // 本地主机ip和端口号
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(HOST_PORT);
    serAddr.sin_addr.s_addr = inet_addr(NATIVE_IP_ADDRESS);
    if (bind(sServer, (struct sockaddr*)&serAddr, sizeof(serAddr)) == -1) {
        printf("bind socket failed\r\n");
        close(sServer);
    }
    // 对方ip和端口号
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(DEVICE_PORT);
    serAddr.sin_addr.s_addr = htons(INADDR_ANY);

    return sServer;
}

void *UdpServerDemo(const char *param)
{
    struct sockaddr_in serAddr = {0};
    struct sockaddr_in remoteAddr = {0};
    static int recvDataFlag = -1;
    char *sendData = NULL;
    int sServer = 0;

    (char*)(param);
    printf(" This Pegasus udp server demo\r\n");
    sServer = UdpTransportInit(serAddr, remoteAddr);

    int addrLen = sizeof(remoteAddr);
    char recvData[UDP_RECV_LEN] = {0};

    while (1) {
        /* 255长度 */
        int recvLen = recvfrom(sServer, recvData, UDP_RECV_LEN, 0, (struct sockaddr*)&remoteAddr, (socklen_t*)&addrLen);
        if (recvLen) {
            if (strstr(inet_ntoa(remoteAddr.sin_addr), INVALID_VALUE) == NULL) {
                printf("A connection was received:%s\r\n", inet_ntoa(remoteAddr.sin_addr));
                printf("Received data:%s\r\n", recvData);
            }
            if (strstr(recvData, WECHAT_MSG_LIGHT_OFF) != NULL) {
                printf("Control equipment information received:%s\r\n", recvData);
                recvDataFlag = HI_FALSE;
                WeChatControlDeviceMsg(HI_GPIO_VALUE1);
            } else if (strstr(recvData, WECHAT_MSG_LIGHT_ON) != NULL) {
                printf("Control equipment information received:%s\r\n", recvData);
                recvDataFlag = HI_TRUE;
                WeChatControlDeviceMsg(HI_GPIO_VALUE0);
            } else if (strstr(recvData, WECHAT_MSG_UNLOAD_PAGE) != NULL) {
                printf("The applet exits the current interface\r\n");
                WeChatControlDeviceMsg(HI_GPIO_VALUE1);
            } else {
                recvDataFlag = RECV_DATA_FLAG_OTHER;
            }
        }
        if (recvDataFlag == HI_TRUE) {
            sendData = DEVICE_MSG_LIGHT_ON;
            sendto(sServer, sendData, strlen(sendData), 0, (struct sockaddr*)&remoteAddr, addrLen);
        } else if (recvDataFlag == HI_FALSE) {
            sendData = DEVICE_MSG_LIGHT_OFF;
            sendto(sServer, sendData, strlen(sendData), 0, (struct sockaddr*)&remoteAddr, addrLen);
        } else if (recvDataFlag == RECV_DATA_FLAG_OTHER) {
            sendData = "Received a message from the server";
            sendto(sServer, sendData, strlen(sendData), 0, (struct sockaddr*)&remoteAddr, addrLen);
        }
        hi_sleep(FREE_CPU_TIME_20MS);
    }
    close(sServer);
    return NULL;
}

#define UDP_TASK_STACKSIZE  0x1000
#define UDP_TASK_PRIOR 27
#define UDP_TASK_NAME "UDP_demo"

static void UDPTransport(void)
{
    osThreadAttr_t attr;
#ifdef CONFIG_WIFI_AP_MODULE
    if (hi_wifi_start_softap() != 0) {
        printf("open softap failure\n");
        return;
    }
    printf("open softap ok\n");
#elif defined(CONFIG_WIFI_STA_MODULE)
    /* start wifi sta module */
    WifiStaModule();
#endif
    attr.name = "udp demo";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = UDP_TASK_STACKSIZE;
    attr.priority = UDP_TASK_PRIOR;

    if (osThreadNew((osThreadFunc_t)UdpServerDemo, NULL, &attr) == NULL) {
        printf("[UDP] Failed to create udp demo!\n");
    }
}

SYS_RUN(UDPTransport);