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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "net_demo.h"
#include "net_common.h"
#include "config_params.h"
#include "wifi_starter.h"
#define TEN 10
#define ONE_HUNDRED 100

int sendMessage = 0;
static char message[256] = "";

void UdpServerTest(unsigned short port)
{
    ssize_t retval = 0;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket

    struct sockaddr_in clientAddr = {0};
    socklen_t clientAddrLen = sizeof(clientAddr);
    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    retval = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (retval < 0) {
        printf("bind failed, %ld!\r\n", retval);
        printf("do_cleanup...\r\n");
        close(sockfd);
    }
    printf("bind to port %u success!\r\n", port);

    while (1) {
        osDelay(TEN);
        retval = recvfrom(sockfd, message, sizeof(message), 0, (struct sockaddr *)&clientAddr, &clientAddrLen);
        printf("recv message %s %ld done!\r\n", message, retval);
        printf("peer info: ipaddr = %s, port = %d\r\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        if (g_wifiStatus == WIFI_AP) {
            int ret = cJSONParseAP(message);
            if (ret < 0) {
                printf("parse message failed\r\n");
            }
            break;
        } else {
            if (retval < 0) {
                printf("recvfrom failed, %ld!\r\n", retval);
                close(sockfd);
            }
            int ret = cJSONParseSTA(message);
            if (ret < 0) {
                printf("parse message failed\r\n");
            }
        }

        if (sendMessage) {
            osDelay(ONE_HUNDRED);
            char *megsend = cJSONReport();
            retval = sendto(sockfd, megsend, strlen(megsend), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
            if (retval <= 0) {
                close(sockfd);
            }
            printf("send message {%s} %ld done!\r\n", megsend, retval);
            sendMessage = 0;
        }
    }
SERVER_TEST_DEMO(UdpServerTest);
