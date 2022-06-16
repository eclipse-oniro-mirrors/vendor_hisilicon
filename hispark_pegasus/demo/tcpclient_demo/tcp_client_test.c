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

#include "net_demo.h"
#include "net_common.h"

static char request[] = "Hello";
static char response[128] = "";

void TcpClientTest(const char* host, unsigned short port)
{
    ssize_t retval = 0;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;  // AF_INET表示IPv4协议
    serverAddr.sin_port = htons(port);  // 端口号，从主机字节序转为网络字节序
    if (inet_pton(AF_INET, host, &serverAddr.sin_addr) <= 0) {  // 将主机IP地址从“点分十进制”字符串 转化为 标准格式（32位整数）
        printf("inet_pton failed!\r\n");
        printf("do_cleanup...\r\n");
        close(sockfd);
    }

    // 尝试和目标主机建立连接，连接成功会返回0 ，失败返回 -1
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("connect failed!\r\n");
        printf("do_cleanup...\r\n");
        close(sockfd);
    }
    printf("connect to server %s success!\r\n", host);

    // 建立连接成功之后，这个TCP socket描述符 —— sockfd 就具有了 “连接状态”，发送、接收 对端都是 connect 参数指定的目标主机和端口
    retval = send(sockfd, request, sizeof(request), 0);
    if (retval < 0) {
        printf("send request failed!\r\n");
        printf("do_cleanup...\r\n");
        close(sockfd);
    }
    printf("send request{%s} %ld to server done!\r\n", request, retval);

    retval = recv(sockfd, &response, sizeof(response), 0);
    if (retval <= 0) {
        printf("send response from server failed or done, %ld!\r\n", retval);
        printf("do_cleanup...\r\n");
        close(sockfd);
    }
    response[retval] = '\0';
    printf("recv response{%s} %ld from server done!\r\n", response, retval);
CLIENT_TEST_DEMO(TcpClientTest);