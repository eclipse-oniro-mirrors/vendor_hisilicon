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
#include <unistd.h>

#include "net_demo.h"
#include "net_common.h"

static char request[] = "Hello.";
static char response[128] = "";

void UdpClientTest(const char* host, unsigned short port)
{
    ssize_t retval = 0;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket

    struct sockaddr_in toAddr = {0};
    toAddr.sin_family = AF_INET;
    toAddr.sin_port = htons(port); // 端口号，从主机字节序转为网络字节序
    if (inet_pton(AF_INET, host, &toAddr.sin_addr) <= 0) { // 将主机IP地址从“点分十进制”字符串 转化为 标准格式（32位整数）
        printf("inet_pton failed!\r\n");
        printf("do_cleanup...\r\n");
        close(sockfd);
    }

    // UDP socket 是 “无连接的” ，因此每次发送都必须先指定目标主机和端口，主机可以是多播地址
    retval = sendto(sockfd, request, sizeof(request), 0, (struct sockaddr *)&toAddr, sizeof(toAddr));
    if (retval < 0) {
        printf("sendto failed!\r\n");
        printf("do_cleanup...\r\n");
        close(sockfd);
    }
    printf("send UDP message {%s} %ld done!\r\n", request, retval);

    struct sockaddr_in fromAddr = {0};
    socklen_t fromLen = sizeof(fromAddr);

    // UDP socket 是 “无连接的” ，因此每次接收时前并不知道消息来自何处，通过 fromAddr 参数可以得到发送方的信息（主机、端口号）
    retval = recvfrom(sockfd, &response, sizeof(response), 0, (struct sockaddr *)&fromAddr, &fromLen);
    if (retval <= 0) {
        printf("recvfrom failed or abort, %ld, %d!\r\n", retval, errno);
        printf("do_cleanup...\r\n");
        close(sockfd);
    }
    response[retval] = '\0';
    printf("recv UDP message {%s} %ld done!\r\n", response, retval);
    printf("peer info: ipaddr = %s, port = %d\r\n", inet_ntoa(fromAddr.sin_addr), ntohs(fromAddr.sin_port));
CLIENT_TEST_DEMO(UdpClientTest);