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

#include "net_common.h"

static char request[128] = "";
void TcpServerTest(unsigned short port)
{
    ssize_t retval = 0;
    int backlog = 1;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    int connfd = -1;

    struct sockaddr_in clientAddr = {0};
    socklen_t clientAddrLen = sizeof(clientAddr);
    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);  // 端口号，从主机字节序转为网络字节序
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // 允许任意主机接入， 0.0.0.0

    retval = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)); // 绑定端口
    if (retval < 0) {
        printf("bind failed, %ld!\r\n", retval);
        close(sockfd);
    }
    printf("bind to port %u success!\r\n", port);

    retval = listen(sockfd, backlog); // 开始监听
    if (retval < 0) {
        printf("listen failed!\r\n");
        printf("do_cleanup...\r\n");
        close(sockfd);
    }
    printf("listen with %d backlog success!\r\n", backlog);

    // 接受客户端连接，成功会返回一个表示连接的 socket ， clientAddr 参数将会携带客户端主机和端口信息 ；失败返回 -1
    // 此后的 收、发 都在 表示连接的 socket 上进行；之后 sockfd 依然可以继续接受其他客户端的连接，
    //  UNIX系统上经典的并发模型是“每个连接一个进程”——创建子进程处理连接，父进程继续接受其他客户端的连接
    //  鸿蒙liteos-a内核之上，可以使用UNIX的“每个连接一个进程”的并发模型
    //     liteos-m内核之上，可以使用“每个连接一个线程”的并发模型
    connfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (connfd < 0) {
        printf("accept failed, %d, %d\r\n", connfd, errno);
        printf("do_cleanup...\r\n");
        close(sockfd);
    }
    printf("accept success, connfd = %d!\r\n", connfd);
    printf("client addr info: host = %s, port = %d\r\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    // 后续 收、发 都在 表示连接的 socket 上进行；
    retval = recv(connfd, request, sizeof(request), 0);
    if (retval < 0) {
        printf("recv request failed, %ld!\r\n", retval);
        printf("do_cleanup...\r\n");
        close(sockfd);
    }
    printf("recv request{%s} from client done!\r\n", request);

    retval = send(connfd, request, strlen(request), 0);
    if (retval <= 0) {
        printf("send response failed, %ld!\r\n", retval);
        printf("do_cleanup...\r\n");
        close(sockfd);
    }
    printf("send response{%s} to client done!\r\n", request);

do_disconnect:
    sleep(1);
    close(connfd);
    sleep(1); // for debug
SERVER_TEST_DEMO(TcpServerTest);