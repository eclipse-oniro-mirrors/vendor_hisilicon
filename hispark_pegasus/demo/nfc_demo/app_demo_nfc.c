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

#include "ssd1306_oled.h"
#include "c081_nfc.h"
#include "iot_i2c.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include "app_demo_config.h"

unsigned char readReg = 0;
#define SEND_CMD_LEN (2)
#define NFC_TASK_SLEEP_1MS     (1)

/* i2c read */
unsigned int WriteRead(unsigned char regHigh8bitCmd, unsigned char regLow8bitCmd,
    unsigned char* recvData, unsigned char sendLen, unsigned char readLen)
{
    IotI2cData co8iNfcI2cReadData = {0};
    IotI2cData c081NfcI2cWriteCmdAddr = {0};

    unsigned char sendUserCmd[SEND_CMD_LEN] = {regHigh8bitCmd, regLow8bitCmd};
    (void)memset_s(&co8iNfcI2cReadData, sizeof(IotI2cData), 0x0, sizeof(IotI2cData));

    c081NfcI2cWriteCmdAddr.sendBuf = sendUserCmd;
    c081NfcI2cWriteCmdAddr.sendLen = sendLen;
    co8iNfcI2cReadData.receiveBuf = recvData;
    co8iNfcI2cReadData.receiveLen = readLen;

    readReg = NFC_CLEAN; // 消除stop信号

    IoTI2cWrite(IOT_I2C_IDX_0, C081_NFC_ADDR & 0xFE,
                c081NfcI2cWriteCmdAddr.sendBuf, c081NfcI2cWriteCmdAddr.sendLen);

    IoTI2cRead(IOT_I2C_IDX_0, C081_NFC_ADDR | I2C_RD,
        co8iNfcI2cReadData.receiveBuf, co8iNfcI2cReadData.receiveLen);
    return 0;
}

#define EEPROM_CMD_1    (0x3B1)
#define EEPROM_CMD_2    (0x3B5)
#define SEND_EEPROM_CMD_LEN (1)
#define SEND_EEPROM_DATA_1  (1)
#define SEND_EEPROM_DATA_2  (3)

/* NFC 芯片配置 ,平时不要调用 NFC init */
void NfcInit(void)
{
    // uint8_t wbuf[5]={0x05,0x72,0xF7,0x60,0x02}; // 芯片默认配置
    unsigned char wBuf[5] = {0x05, 0x78, 0xF7, 0x90, 0x02}; // 芯片默认配置
    /* 读取字节的时候屏蔽csn引脚,写eep的时候打开 */
    IoSetFunc(IOT_IO_NAME_GPIO9, IOT_IO_FUNC_GPIO_9_GPIO);
    IoTGpioSetDir(IOT_GPIO_IDX_9, IOT_GPIO_DIR_OUT);
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO9, IOT_GPIO_VALUE0);

    Fm11WriteEep(EEPROM_CMD_1, SEND_EEPROM_CMD_LEN, &wBuf[SEND_EEPROM_DATA_1]); /* send EEPROM cmd */
    Fm11WriteEep(EEPROM_CMD_2, SEND_EEPROM_CMD_LEN, &wBuf[SEND_EEPROM_DATA_2]); /* send EEPROM cmd */
}

void *NfcTask(const char* param)
{
    (void) param;
    IoTGpioInit(IOT_IO_NAME_GPIO_13);
    IoSetFunc(IOT_IO_NAME_GPIO_13, IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoTGpioInit(IOT_IO_NAME_GPIO_14);
    IoSetFunc(IOT_IO_NAME_GPIO_14, IOT_IO_FUNC_GPIO_14_I2C0_SCL);

    IoTI2cInit(IOT_I2C_IDX_0, HI_I2C_IDX_BAUDRATE); // baud 400k
    IoTI2cSetBaudrate(IOT_I2C_IDX_0, HI_I2C_IDX_BAUDRATE);
    printf("nfc task\r\n");
    NfcRead();
}

/* c08i nfc task */
void NfcExampleEntry(void)
{
    osThreadAttr_t attr = {0};

    attr.stack_size = C08I_NFC_DEMO_TASK_STAK_SIZE;
    attr.priority = C08I_NFC_TASK_PRIORITY;
    attr.name = "nfcTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;

    if (osThreadNew((osThreadFunc_t)NfcTask, NULL, &attr) == NULL) {
        printf("[nfcExample] Falied to create LedTask!\n");
    }
}

SYS_RUN(NfcExampleEntry);

/* nfc display */
void *AppNfcDisplay(char* param)
{
    for (; ;) {
        OledNfcDisplay();
        TaskMsleep(NFC_TASK_SLEEP_1MS);
    }
}

/* nfc display task */
void NfcDisplayExampleEntry(void)
{
    osThreadAttr_t attr = {0};
    attr.stack_size = NFC_DISPLAY_TASK_STAK_SIZE;
    attr.priority = C08I_NFC_DEMO_TASK_PRIORITY;
    attr.name = "app_nfc_display";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    if (osThreadNew((osThreadFunc_t)AppNfcDisplay, NULL, &attr) == NULL) {
        printf("[nfcDisplayExampleEntry] Falied to create LedTask!\n");
    }
}

SYS_RUN(NfcDisplayExampleEntry);