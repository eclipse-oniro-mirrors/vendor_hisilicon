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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "lwip/netifapi.h"

#define CLIENT_SOCKET_CREATE_FAILED (-1)
#define UDP_SERVER_PORT (6655) /* TEST Port */
#define UDP_CLIENT_PORT (5566) /* TEST Port */

#define UDP_RECV_BUF_LEN (255)

int UdpClientDemo(void)
{
    struct sockaddr_in sin = {0};
    struct sockaddr_in serverAddr = {0};
    /* create socket */
    int sClient = socket(AF_INET, SOCK_DGRAM, 0);
    if (sClient == CLIENT_SOCKET_CREATE_FAILED) {
        printf("create socket Fialed\r\n");
        close(sClient);
        return CLIENT_SOCKET_CREATE_FAILED;
    }
    // 本地主机ip和端口号
    sin.sin_family = AF_INET;
    sin.sin_port = htons(UDP_SERVER_PORT); /* server端口号6655 */
    sin.sin_addr.s_addr = inet_addr("XXX.XXX.X.XXX");
    int ret = bind(sClient, (struct sockaddr*)&sin, sizeof(sin));
    if (ret < 0) {
        printf("client bind failed %d, %s\r\n", __LINE__, errno);
    }
    // 对方ip和端口号
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(UDP_CLIENT_PORT); /* client端口号5566 */
    serverAddr.sin_addr.s_addr = inet_addr("XXXX.XXX.X.XXX");

    int len = sizeof(sin);
    char *sendData = "这是客户端的消息\r\n";
    while (1) {
        sendto(sClient, sendData, strlen(sendData), 0, (struct sockaddr*)&serverAddr, len);
        char recvData[UDP_RECV_BUF_LEN] = {0};
        /* buff大小设置为255 */
        int recvLen = recvfrom(sClient, recvData, UDP_RECV_BUF_LEN, 0, (struct sockaddr*)&serverAddr, &len);
        if (recvLen) {
            printf("recv_data = %s\r\n", recvData);
        }
    }
    close(sClient);
    return 0;
}