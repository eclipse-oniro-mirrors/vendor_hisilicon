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

#include <unistd.h>
#include "ssd1306_oled.h"
#include "c081_nfc.h"
#include "iot_i2c.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include "app_demo_config.h"

#define NDEF_FILE_LEN (1024)
#define TASK_SLEEP_10MS (10)
#define TASK_SLEEP_1MS (1)
#define FIFO_LC_CMD (2)

T4T_FILE currentFile;

unsigned char capabilityContainer[15] = {
    0x00, 0x0F,        // CCLEN
    0x20,              // Mapping Version
    0x00, 0xF6,        // MLe 必须是F6  写成FF超过256字节就会分帧  但是写成F6就不会分帧
    0x00, 0xF6,        // MLc 必须是F6  写成FF超过256字节就会分帧  但是写成F6就不会分帧
    0x04,              // NDEF消息格式 05的话就是私有
    0x06,              // NDEF消息长度
    0xE1, 0x04,        // NDEF FILE ID       NDEF的文件标识符
    0x03, 0x84,        // NDEF最大长度
    0x00,              // Read Access           可读
    0x00               // Write Access          可写
};
unsigned char ndefFile[NDEF_FILE_LEN] = {
/* wechat ndef */
#ifdef  NFC_TAG_WECHAT
    0x00, 0x20,
    0xd4, 0x0f, 0x0e, 0x61, 0x6e, 0x64, 0x72, 0x6f,
    0x69, 0x64, 0x2e, 0x63, 0x6f, 0x6d, 0x3a, 0x70,
    0x6b, 0x67, 0x63, 0x6f, 0x6d, 0x2e, 0x74, 0x65,
    0x6e, 0x63, 0x65, 0x6e, 0x74, 0x2e, 0x6d, 0x6d,
#endif
};
unsigned char fm327Fifo[NDEF_FILE_LEN];
unsigned char irqTxdone = 0;
unsigned char rfLen = 0;
unsigned char irqRxdone = 0;
unsigned char FlagFirstFrame = 0; // 卡片首帧标识

void SetNdefData(void)
{
    (void)memset_s(ndefFile, sizeof(ndefFile), 0x00, sizeof(ndefFile));
}

unsigned char GetNdefData(unsigned char *ndefFileData, unsigned int ndefLen)
{
    int ret = 0;
    ret = memcpy_s(ndefFile, NDEF_FILE_LEN, ndefFileData, ndefLen);
    if (ret != 0) {
        return -1;
    }
    return 0;
}

/* co8i 写命令: 该接口写eeprom 更改芯片配置 */
unsigned char C08iNfcI2cWrite(unsigned char regHigh8bitCmd, unsigned char regLow8bitCmd,
    unsigned char* dataBuff, unsigned char len)
{
    IotI2cIdx id = IOT_I2C_IDX_0;
    IotI2cData C081nfcI2cWriteCmdAddr = {0};
    unsigned char sendUserCmd[64] = {regHigh8bitCmd, regLow8bitCmd};

    C081nfcI2cWriteCmdAddr.sendBuf = sendUserCmd;
    C081nfcI2cWriteCmdAddr.sendLen = 2 + len; /* 2: send lenght */
    for (int i = 0; i < len; i++) {
        sendUserCmd[2 + i] = *(dataBuff + i); /* 2: send lenght */
    }
    IoTI2cWrite(id,
                C081_NFC_ADDR& 0xFE,
                C081nfcI2cWriteCmdAddr.sendBuf,
                C081nfcI2cWriteCmdAddr.sendLen);

    return 0;
}

/* 写寄存器 */
unsigned int WriteFifoReg(unsigned char regHigh8bitCmd,
    unsigned char regLow8bitCmd, unsigned char dataBuff)
{
    IotI2cIdx id = IOT_I2C_IDX_0;
    IotI2cData c081nfcI2cWriteCmdAddr = {0};
    unsigned char sendUserCmd[3] = {regHigh8bitCmd, regLow8bitCmd, dataBuff};
    c081nfcI2cWriteCmdAddr.sendBuf = sendUserCmd;
    c081nfcI2cWriteCmdAddr.sendLen = 3; /* 3: send lenght */
    IoTI2cWrite(id,
                C081_NFC_ADDR & 0xFE,
                c081nfcI2cWriteCmdAddr.sendBuf,
                c081nfcI2cWriteCmdAddr.sendLen);
    return 0;
}

/* 写fifo data */
unsigned int WriteFifoData(unsigned char* dataBuff, unsigned char len)
{
    unsigned char* writeBuf = dataBuff;

    IotI2cIdx id = IOT_I2C_IDX_0;
    IotI2cData c081nfcI2cWriteCmdAddr = {0};
    unsigned char sendUserCmd[128] = {0};

    (void)memset_s(sendUserCmd, sizeof(sendUserCmd), 0x0, sizeof(sendUserCmd));
    sendUserCmd[0] = 0xff;
    sendUserCmd[1] = 0xf0;
    for (int i = 0; i < len; i++) {
        sendUserCmd[2 + i] = *(writeBuf + i); /* 2: buffer index address */
    }
    c081nfcI2cWriteCmdAddr.sendBuf = sendUserCmd;
    c081nfcI2cWriteCmdAddr.sendLen = 2 + len; /* 2: send lenght */
    IoTI2cWrite(id,
                C081_NFC_ADDR & 0xFE,
                c081nfcI2cWriteCmdAddr.sendBuf,
                c081nfcI2cWriteCmdAddr.sendLen);
    return 0;
}
#define DELAY_10MS (10000)
/* EEPROM page write */
void EepWritePage(unsigned char *pBuffer, unsigned short WriteAddr, unsigned char dataLen)
{
    C08iNfcI2cWrite((unsigned char)((WriteAddr & 0xFF00) >> 8), /* 8: right move 8 bit */
                    (unsigned char)(WriteAddr & 0x00FF),
                    pBuffer,
                    dataLen);
    hi_udelay(DELAY_10MS); // 必须延时10ms
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO9, IOT_GPIO_VALUE1);
    printf("----- EepWritePage %d ! -----\r\n\n", __LINE__);
}

/* 写EEPROM */
void Fm11WriteEep(unsigned short addr, unsigned int len, unsigned char *wbuf)
{
    unsigned char offset;
    unsigned short address = addr;
    unsigned char *writeBuf = wbuf;
    unsigned int lenght = len;

    if (address < FM11_E2_USER_ADDR || address >= FM11_E2_MANUF_ADDR) {
        return;
    }
    if (address % FM11_E2_BLOCK_SIZE) {
        offset = FM11_E2_BLOCK_SIZE - (address % FM11_E2_BLOCK_SIZE);
        if (lenght > offset) {
            EepWritePage(writeBuf, address, offset);
            address += offset;
            writeBuf += offset;
            lenght -= offset;
        } else {
            EepWritePage(writeBuf, address, lenght);
            lenght = 0;
        }
    }
    while (lenght) {
        if (lenght >= FM11_E2_BLOCK_SIZE) {
            EepWritePage(writeBuf, address, FM11_E2_BLOCK_SIZE);
            address += FM11_E2_BLOCK_SIZE;
            writeBuf += FM11_E2_BLOCK_SIZE;
            lenght -= FM11_E2_BLOCK_SIZE;
        } else {
            EepWritePage(writeBuf, address, lenght);
            lenght = 0;
        }
    }
}

/* 读EEPROM */
unsigned int Fm11ReadEep(unsigned char *dataBuff, unsigned short ReadAddr, unsigned short len)
{
    WriteRead((unsigned char)((ReadAddr & 0xFF00) >> 8), /* 8: right move 8 bit */
        (unsigned char)(ReadAddr & 0x00FF),
        dataBuff,
        2, /* 2: read lenght */
        len);

    return  0;
}

/* 读NFC寄存器 */
unsigned char Fm11ReadReg(unsigned short addr)
{
    unsigned char pdata[10] = {0};
    unsigned char a = 0;

    if (Fm11ReadEep(pdata, addr, 1) == 0) { /* 1: read lenght */
        a = pdata[0];
        return a;
    } else {
        printf("fm11_read_eep failed \r\n");
        return -1;
    }
}

/* 写NFC寄存器 */
unsigned char Fm11WriteReg(unsigned short addr, unsigned char data)
{
    unsigned int status = 0;

    status = WriteFifoReg((unsigned char)((addr & 0xFF00) >> 8), /* 8: right move 8 bit */
        (unsigned char)(addr & 0x00FF), data);
    if (status != 0) {
        return -1;
    }
    return 0;
}

/* 读取FIFO */
unsigned char  Fm11ReadFifo(unsigned char NumByteToRead, unsigned char *pbuf)
{
    unsigned char readFifoLen = NumByteToRead;

    if (Fm11ReadEep(pbuf, FM327_FIFO, readFifoLen) == 0) {
        return 0;
    } else {
        return -1;
    }
}

/* 写FIFO */
unsigned char Fm11WriteFifo(unsigned char *pbuf, unsigned char len)
{
    unsigned char status;

    if (pbuf == NULL) {
        return -1;
    }
    status = WriteFifoData(pbuf, len);
    if (status != 0) {
        return -1;
    }
    return 0;
}

void NfcDataRead(unsigned char *sBuf, unsigned int sLen)
{
    unsigned int sendLen = sLen;
    unsigned char *sendBuf = sBuf;

    Fm11WriteFifo(sendBuf, 32); // 32: write fifo 先发32字节进fifo
    Fm11WriteReg(RF_TXEN_REG, 0x55); // 写0x55时触发非接触口回发数据

    sendLen = sendLen - 32; // 32: 待发长度－32
    sendBuf = sendBuf + 32; // 32: 待发数据指针+32

    while (sendLen > 0) {
        if ((Fm11ReadReg(FIFO_WORDCNT_REG) & 0x3F) <= 8) { /* 8: read reg below 8 */
            if (sendLen <= 24) { /* 24: read 24 Byte */
                Fm11WriteFifo(sendBuf, sendLen); // write fifo 先发32字节进fifo
                sendLen = 0;
            } else {
                Fm11WriteFifo(sendBuf, 24); // 24: write fifo 先发24字节进fifo
                sendLen = sendLen - 24; // 24: 待发长度－24
                sendBuf = sendBuf + 24; // 24: 待发数据指针+24
            }
        }
    }
    irqTxdone = 0;
}

/* 数据回发 */
void Fm11DataSend(unsigned int iLen, unsigned char *iBuf)
{
    unsigned int sLen;
    unsigned char *sBuf = NULL;
    unsigned int inLen = iLen;

    if (iBuf == NULL) {
        return;
    }
    sLen = inLen;
    sBuf = &iBuf[0];

    if (sLen <= 32) { /* 32: send data lenght */
        Fm11WriteFifo(sBuf, sLen); // write fifo 有多少发多少
        sLen = 0;
        Fm11WriteReg(RF_TXEN_REG, 0x55); // 写0x55时触发非接触口回发数据
    } else {
        NfcDataRead(sBuf, sLen);
    }
}

unsigned int Fm11DataRecvInterrupt(unsigned char *rbuf)
{
    unsigned char irq = 0;
    unsigned char ret = 0;
    unsigned char irqDataWl = 0;
    unsigned char irqDataIn = 0;
    unsigned int rlen = 0;
    unsigned int temp = 0;

    while (1) {
        irqDataWl = 0;
        irq = Fm11ReadReg(MAIN_IRQ); // 查询中断标志
        if (irq & MAIN_IRQ_FIFO) {
            ret = Fm11ReadReg(FIFO_IRQ);
            if (ret & FIFO_IRQ_WL) {
                irqDataWl = 1;
            }
        }
        if (irq & MAIN_IRQ_AUX) {
            Fm11ReadReg(AUX_IRQ);
            Fm11WriteReg(FIFO_FLUSH, 0xFF);
        }
        if (irq & MAIN_IRQ_RX_START) {
            irqDataIn = 1; /* read reg flag */
        }
        if (irqDataIn && irqDataWl) {
            irqDataWl = 0;
            Fm11ReadFifo(24, &rbuf[rlen]); // 24: 渐满之后读取24字节
            rlen += 24; // 24: 渐满之后读取24字节
        }
        if (irq & MAIN_IRQ_RX_DONE) {
            temp = (unsigned int)(Fm11ReadReg(FIFO_WORDCNT) & 0x3F); // 0x3F: 接收完全之后，查fifo有多少字节
            Fm11ReadFifo(temp, &rbuf[rlen]); // 读最后的数据
            rlen += temp;
            irqDataIn = 0;
            break;
        }
        TaskMsleep(TASK_SLEEP_10MS);
    }

    if (rlen <= 2) { // 2: 字节crc校验
        return 0;
    }
    rlen -= 2; // 2: 字节crc校验
    return rlen;
}

/* 读取RF数据 */
unsigned int Fm11DataRecv(unsigned char *rbuf)
{
    unsigned char irq = 0;
    unsigned char ret = 0;
    unsigned char irqDataWl = 0;
    unsigned char irqDataIn = 0;
    unsigned int rlen = 0;
    unsigned int temp = 0;

    /* 查询方式 */
    while (1) {
        irqDataWl = 0;
        irq = Fm11ReadReg(MAIN_IRQ); // 查询中断标志
        if (irq & MAIN_IRQ_FIFO) {
            ret = Fm11ReadReg(FIFO_IRQ);
            if (ret & FIFO_IRQ_WL) {
                irqDataWl = 1; /* reg read flag */
            }
        }
        if (irq & MAIN_IRQ_AUX) {
            Fm11ReadReg(AUX_IRQ);
            Fm11WriteReg(FIFO_FLUSH, 0xFF);
        }
        if (irq& MAIN_IRQ_RX_START) {
            irqDataIn = 1; /* Data read flag */
        }
        if (irqDataIn && irqDataWl) {
            irqDataWl = 0;
            Fm11ReadFifo(24, &rbuf[rlen]); // 24: 渐满之后读取24字节
            rlen += 24; // 24: 渐满之后读取24字节
        }
        if (irq & MAIN_IRQ_RX_DONE) {
            temp =(unsigned int)(Fm11ReadReg(FIFO_WORDCNT) & 0x3F); // 0x3F: 接收完全之后，查fifo有多少字节
            Fm11ReadFifo(temp, &rbuf[rlen]); // 读最后的数据
            rlen += temp;
            irqDataIn = 0;
            break;
        }
        TaskMsleep(TASK_SLEEP_10MS);
    }

    if (rlen <= 2) { // 2: 字节crc校验
        return 0;
    }
    rlen -= 2; // 2: 字节crc校验
    return rlen;
}

void SetApdu(unsigned char *statusOk, unsigned char *statusWord2,
    const unsigned char *ndefCapabilityContainer,
    const unsigned char *ndefId)
{
    if (fm327Fifo[P1] == 0x00) { /* 0x00:  FIFO P1 cmd */
        if ((fm327Fifo[LC] == FIFO_LC_CMD) &&
            (memcmp(ndefCapabilityContainer, fm327Fifo + DATA, fm327Fifo[LC]) == 0)) {
            Fm11WriteFifo(statusOk, 3); /* 3: statusOk lenght */
            Fm11WriteReg(RF_TXEN_REG, 0x55); /* 0x55: TX reg cmd */
            currentFile = CC_FILE;
        } else if ((fm327Fifo[LC] == FIFO_LC_CMD) &&
            (memcmp(ndefId, fm327Fifo + DATA, fm327Fifo[LC]) == 0)) {
            Fm11WriteFifo(statusOk, 3); /* 3: statusOk lenght */
            Fm11WriteReg(RF_TXEN_REG, 0x55); /* TX reg cmd */
            currentFile = NDEF_FILE;
        } else {
            Fm11WriteFifo(statusWord2, 3); /* 3: statusOk lenght */
            Fm11WriteReg(RF_TXEN_REG, 0x55); /* TX reg cmd */
            currentFile = NONE;
        }
    } else if (fm327Fifo[P1] == 0x04) { /* 0x04:  FIFO P1 cmd */
        Fm11WriteFifo(statusOk, 3); /* 3: statusOk lenght */
        Fm11WriteReg(RF_TXEN_REG, 0x55); /* 0x55: TX reg cmd */
    } else {
        Fm11WriteFifo(statusOk, 3); /* 3: statusOk lenght */
        Fm11WriteReg(RF_TXEN_REG, 0x55); /* 0x55: TX reg cmd */
    }
}

void SelectApdu(unsigned char *statusOk,
                unsigned char *statusWord,
                unsigned char *statusWord2,
                const unsigned char *ndefCapabilityContainer,
                const unsigned char *ndefId)
{
    unsigned char xlen;
    unsigned char xbuf[256] = {0};

    if (fm327Fifo[INS] == 0xA4) { /* 0xA4:  FIFO INS cmd */
        SetApdu(statusOk,
                statusWord2,
                ndefCapabilityContainer,
                ndefId);
    } else if (fm327Fifo[INS] == 0xB0) { /* 0xB0:  FIFO INS cmd */
        if (currentFile == CC_FILE) {
            Fm11WriteFifo(statusOk, 1); /* 1: statusOk lenght */
            Fm11WriteFifo(capabilityContainer + (fm327Fifo[P1] << 8) + fm327Fifo[P2], /* 8: left move 8 bit */
                fm327Fifo[LC]);
            Fm11WriteFifo(&statusOk[1], 2); /* 2: statusOk lenght */
            Fm11WriteReg(RF_TXEN_REG, 0x55); /* 0x55: TX reg cmd */
        } else if (currentFile == NDEF_FILE) {
            (void)memcpy_s(&xbuf[0], NDEF_FILE_LEN, &statusOk[0], 1); /* 1: statusOk lenght */
            (void)memcpy_s(&xbuf[1], /* 1: xbuf lenght */
                NDEF_FILE_LEN,
                &ndefFile[0] + (fm327Fifo[P1] << 8) + fm327Fifo[P2], /* 8: left move 8 bit */
                fm327Fifo[LC]);
            (void)memcpy_s(&xbuf[0] + fm327Fifo[LC] + 1, /* 1: compy data addr */
                NDEF_FILE_LEN, statusOk + 1, /* 1: compy data addr */
                2); /* 2:  compy data lenght */
            xlen = fm327Fifo[LC] + 3; /* 3:  FIFO LC lenght */
            Fm11DataSend(xlen, xbuf);
        } else {
            Fm11WriteFifo(statusWord, 3); /* 3: statusOk lenght */
            Fm11WriteReg(RF_TXEN_REG, 0x55); /* 0x55: TX reg cmd */
        }
    } else if (fm327Fifo[INS] ==  0xD6) { // UPDATE_BINARY
        (void)memcpy_s(ndefFile + (fm327Fifo[P1] << 8) + fm327Fifo[P2], /* 8: left move 8 bit */
            NDEF_FILE_LEN, fm327Fifo + DATA,
            fm327Fifo[LC]);
        Fm11WriteFifo(statusOk, 3); /* 3: statusOk lenght */
        Fm11WriteReg(RF_TXEN_REG, 0x55); /* 0x55: TX reg cmd */
    } else {
        Fm11DataSend(rfLen, fm327Fifo);
    }
}

/* 写fifo 和 写寄存器 */
void Fm11T4t(void)
{
    unsigned char nakCrcErr = 0x05;
    unsigned char crcErr = 0;
    unsigned char statusOk[3] = { 0x02, 0x90, 0x00 };
    unsigned char statusWord[3] = { 0x02, 0x6A, 0x82 };
    unsigned char statusWord2[3] = { 0x02, 0x6A, 0x00 };
    const unsigned char ndefCapabilityContainer[2] = { 0xE1, 0x03 };
    const unsigned char ndefId[2] = { 0xE1, 0x04 };

    if (crcErr) {
        Fm11WriteFifo(&nakCrcErr, 1); /* 1: write FiFo lenght */
        Fm11WriteReg(RF_TXEN_REG, 0x55); /* 0x55: TX reg cmd */
        crcErr = 0;
    } else {
        statusOk[0] = fm327Fifo[0];
        statusWord[0] = fm327Fifo[0];
        statusWord2[0] = fm327Fifo[0];
        SelectApdu(statusOk, statusWord, statusWord2,
            ndefCapabilityContainer, ndefId);
    }
}

/* app nfc demo */
void NfcRead(void)
{
#ifdef  CHECK
    while (1) {
        rfLen = Fm11DataRecv(fm327Fifo); // 读取rf数据(一帧)
        if (rfLen > 0) {
            Fm11T4t();
        }
        TaskMsleep(TASK_SLEEP_1MS);
    }
#endif

#ifdef NFC_INTERRUPT
    while (1) {
        if (FlagFirstFrame) {
            rfLen = Fm11DataRecvInterrupt(fm327Fifo); // 读取rf数据(一帧)
            if (rfLen > 0) {
                fm11_t4t();
            }
        }
        TaskMsleep(TASK_SLEEP_1MS);
    }
#endif
}