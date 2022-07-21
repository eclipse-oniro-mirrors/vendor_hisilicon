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


#include <stddef.h>
#include <stdio.h>
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_i2c.h"
#include "wifiiot_errno.h"
#include "oled_fonts.h"
#include "oled_ssd1306.h"
#define OLED_I2C_IDX WIFI_IOT_I2C_IDX_0

#define OLED_WIDTH    (128)
#define OLED_I2C_ADDR 0x78 // 默认地址为 0x78
#define OLED_I2C_CMD 0x00 // 0000 0000       写命令
#define OLED_I2C_DATA 0x40 // 0100 0000(0x40) 写数据
#define OLED_I2C_BAUDRATE (400*1000) // 400k

#define DELAY_100_MS (100*1000)
#define FOUR 4
#define EIGHT 8
#define TWO 2
#define SIX 6
#define SIXTEEN 16
#define ONE_HUNDRED_AND_TWENTY 120
#define ONE_HUNDRED_AND_TWENTY_EIGHT 128

static uint32_t I2cWiteByte(uint8_t regAddr, uint8_t byte)
{
    WifiIotI2cIdx id = OLED_I2C_IDX;
    uint8_t buffer[] = {regAddr, byte};
    WifiIotI2cData i2cData = {0};

    i2cData.sendBuf = buffer;
    i2cData.sendLen = sizeof(buffer)/sizeof(buffer[0]);

    return I2cWrite(id, OLED_I2C_ADDR, &i2cData);
}

/**
 * @brief Write a command byte to OLED device.
 *
 * @param cmd the commnad byte to be writen.
 * @return Returns {@link WIFI_IOT_SUCCESS} if the operation is successful;
 * returns an error code defined in {@link wifiiot_errno.h} otherwise.
 */
static uint32_t WriteCmd(uint8_t cmd)
{
    return I2cWiteByte(OLED_I2C_CMD, cmd);
}

/**
 * @brief Write a data byte to OLED device.
 *
 * @param cmd the data byte to be writen.
 * @return Returns {@link WIFI_IOT_SUCCESS} if the operation is successful;
 * returns an error code defined in {@link wifiiot_errno.h} otherwise.
 */
static uint32_t WriteData(uint8_t data)
{
    return I2cWiteByte(OLED_I2C_DATA, data);
}

/**
 * @brief ssd1306 OLED Initialize.
 */
uint32_t OledInit(void)
{
    static const uint8_t initCmds[] = {
        0xAE, // --display off
        0x00, // ---set low column address
        0x10, // ---set high column address
        0x40, // --set start line address
        0xB0, // --set page address
        0x81, // contract control
        0xFF, // --128
        0xA1, // set segment remap
        0xA6, // --normal / reverse
        0xA8, // --set multiplex ratio(1 to 64)
        0x3F, // --1/32 duty
        0xC8, // Com scan direction
        0xD3, // -set display offset
        0x00, //
        0xD5, // set osc division
        0x80, //
        0xD8, // set area color mode off
        0x05, //
        0xD9, // Set Pre-Charge Period
        0xF1, //
        0xDA, // set com pin configuration
        0x12, //
        0xDB, // set Vcomh
        0x30, //
        0x8D, // set charge pump enable
        0x14, //
        0xAF, // --turn on oled panel
    };

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_13, WIFI_IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_IO_FUNC_GPIO_14_I2C0_SCL);

    I2cInit(WIFI_IOT_I2C_IDX_0, OLED_I2C_BAUDRATE);

    for (size_t i = 0; i < sizeof(initCmds)/sizeof(initCmds[0]); i++) {
        uint32_t status = WriteCmd(initCmds[i]);
        if (status != WIFI_IOT_SUCCESS) {
            return status;
        }
    }
    return WIFI_IOT_SUCCESS;
}

void OledSetPosition(uint8_t x, uint8_t y)
{
    WriteCmd(0xb0 + y);
    WriteCmd(((x & 0xf0) >> FOUR) | 0x10);
    WriteCmd(x & 0x0f);
}

void OledFillScreen(uint8_t fillData)
{
    uint8_t m = 0;
    uint8_t n = 0;

    for (m=0; m < EIGHT; m++) {
        WriteCmd(0xb0 + m);
        WriteCmd(0x00);
        WriteCmd(0x10);

        for (n=0; n < ONE_HUNDRED_AND_TWENTY_EIGHT; n++) {
            WriteData(fillData);
        }
    }
}

/**
 * @brief 8*16 typeface
 * @param x: write positon start from x axis
 * @param y: write positon start from y axis
 * @param ch: write data
 * @param font: selected font
 */
void OledShowChar(uint8_t x, uint8_t y, uint8_t ch, Font font)
{
    uint8_t c = 0;
    uint8_t i = 0;
    int num_x = x;
    int num_y = y;

    c = ch - ' '; // 得到偏移后的值
    if (x > OLED_WIDTH - 1) {
        num_x = 0;
        num_y = y + TWO;
    }

    if (font == FONT8x16) {
        OledSetPosition(x, y);
        for (i = 0; i < EIGHT; i++) {
            WriteData(F8X16[c*SIXTEEN + i]);
        }

        OledSetPosition(x, y+1);
        for (i = 0; i < EIGHT; i++) {
            WriteData(F8X16[c*SIXTEEN + i + EIGHT]);
        }
    } else {
        OledSetPosition(x, y);
        for (i = 0; i < SIX; i++) {
            WriteData(F6x8[c][i]);
        }
    }
}

void OledShowString(uint8_t x, uint8_t y, const char* str, Font font)
{
    uint8_t j = 0;
    int num_x = x;
    int num_y = y;
    if (str == NULL) {
        printf("param is NULL,Please check!!!\r\n");
        return;
    }

    while (str[j]) {
        OledShowChar(x, y, str[j], font);
    num_x += EIGHT;
    if (x > ONE_HUNDRED_AND_TWENTY) {
            num_x = 0;
            num_y += TWO;
        }
    j++;
    }
}
