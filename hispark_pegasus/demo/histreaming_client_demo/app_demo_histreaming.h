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

#ifndef APP_DEMO_HISTREAMING_H
#define APP_DEMO_HISTREAMING_H

#define UART_BUFF_SIZE           32
#define HIGH_NUM    2
#define HIGH_ASCII   '9'
#define HIGH_NUM2   48
#define HIGH_NUM3   7
#define HIGH_NUM4   4

typedef enum {
    UART_RECEIVE_FLAG = 0,
    UART_RECVIVE_LEN,
    UART_SEND_FLAG = 2,
    UART_SEND_LEN
}UartDefType;

typedef enum {
    UART_RECV_TRUE = 0,
    UART_RECV_FALSE,
}UartRecvDef;

typedef struct {
    unsigned int uartChannel;
    unsigned char g_receiveUartBuff[UART_BUFF_SIZE];
    int g_uartReceiveFlag;
    int g_uartLen;
}UartDefConfig;

void *HistreamingOpen(void);
void HistreamingClose(const char *link);
int SetUartRecvFlag(UartRecvDef def);
int GetUartConfig(UartDefType type);
void ResetUartReceiveMsg(void);
unsigned char *GetUartReceiveMsg(void);

#endif