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

#include "ssd1306.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>  // For memcpy

#include <stdio.h>
#include <unistd.h>
#include "cmsis_os2.h"
#include "iot_i2c.h"
#include "iot_errno.h"

#if defined(SSD1306_USE_I2C)

#define SSD1306_I2C_IDX 0

#define SSD1306_CTRL_CMD 0x00
#define SSD1306_CTRL_DATA 0x40
#define SSD1306_MASK_CONT (0x1<<7)
#define DOUBLE 2

typedef struct {
    /** Pointer to the buffer storing data to send */
    unsigned char *sendBuf;
    /** Length of data to send */
    unsigned int sendLen;
    /** Pointer to the buffer for storing data to receive */
    unsigned char *receiveBuf;
    /** Length of data received */
    unsigned int receiveLen;
} WifiIotI2cData;


void ssd1306_Reset(void)
{
    /* for I2C - do nothing */
}

void HAL_Delay(uint32_t ms)
{
    uint32_t t = 1000;
    uint32_t msPerTick = t / osKernelGetTickFreq(); // 10ms
    if (ms >= msPerTick) {
        osDelay(ms / msPerTick);
    }

    uint32_t restMs = ms % msPerTick;
    if (restMs > 0) {
        usleep(restMs * t);
    }
}

uint32_t HAL_GetTick(void)
{
    uint32_t t = 1000;
    uint32_t msPerTick = t / osKernelGetTickFreq(); // 10ms
    uint32_t tickMs = osKernelGetTickCount()* msPerTick;

    uint32_t csPerMs = osKernelGetSysTimerFreq()/ t; // 160K cycle/ms
    uint32_t csPerTick = csPerMs * msPerTick; // 1600K cycles/tick
    uint32_t restMs = osKernelGetSysTimerCount()% csPerTick / csPerMs;

    return tickMs + restMs;
}

static uint32_t ssd1306_SendData(uint8_t* data, size_t size)
{
    unsigned int id = SSD1306_I2C_IDX;
    WifiIotI2cData i2cData = {0};

    i2cData.sendBuf = data;
    i2cData.sendLen = size;

    uint32_t retval = IoTI2cWrite(id, SSD1306_I2C_ADDR, i2cData.sendBuf, i2cData.sendLen);
    if (retval != IOT_SUCCESS) {
        printf("I2cWrite(%02X)failed, %0X!\n", i2cData.sendBuf[0], retval);
        return retval;
    }
    return IOT_SUCCESS;
}

static uint32_t ssd1306_WiteByte(uint8_t regAddr, uint8_t byte)
{
    uint8_t buffer[] = {regAddr, byte};
    return ssd1306_SendData(buffer, sizeof(buffer));
}

// Send a byte to the command register
void ssd1306_WriteCommand(uint8_t byte)
{
    ssd1306_WiteByte(SSD1306_CTRL_CMD, byte);
}

// Send data
void ssd1306_WriteData(uint8_t* buffer, size_t buff_size)
{
    uint8_t data[SSD1306_WIDTH * DOUBLE] = {0};
    for (size_t i = 0; i < buff_size; i++) {
        data[i*DOUBLE] = SSD1306_CTRL_DATA | SSD1306_MASK_CONT;
        data[i*DOUBLE+1] = buffer[i];
    }
    data[(buff_size - 1)* DOUBLE] = SSD1306_CTRL_DATA;
    ssd1306_SendData(data, sizeof(data));
}

#elif defined(SSD1306_USE_SPI)

void ssd1306_Reset(void)
{
    unsigned int time = 10;
    // CS = High (not selected)
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_SET);

    // Reset the OLED
    HAL_GPIO_WritePin(SSD1306_Reset_Port, SSD1306_Reset_Pin, GPIO_PIN_RESET);
    HAL_Delay(time);
    HAL_GPIO_WritePin(SSD1306_Reset_Port, SSD1306_Reset_Pin, GPIO_PIN_SET);
    HAL_Delay(time);
}

// Send a byte to the command register
void ssd1306_WriteCommand(uint8_t byte)
{
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_RESET); // select OLED
    HAL_GPIO_WritePin(SSD1306_DC_Port, SSD1306_DC_Pin, GPIO_PIN_RESET); // command
    HAL_SPI_Transmit(&SSD1306_SPI_PORT, (uint8_t *)&byte, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_SET); // un-select OLED
}

// Send data
void ssd1306_WriteData(uint8_t* buffer, size_t buff_size)
{
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_RESET); // select OLED
    HAL_GPIO_WritePin(SSD1306_DC_Port, SSD1306_DC_Pin, GPIO_PIN_SET); // data
    HAL_SPI_Transmit(&SSD1306_SPI_PORT, buffer, buff_size, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_SET); // un-select OLED
}

#else
#error "You should define SSD1306_USE_SPI or SSD1306_USE_I2C macro"
#endif

// Screenbuffer
static uint8_t SSD1306_Buffer[SSD1306_BUFFER_SIZE];

// Screen object
static SSD1306_t SSD1306;

/* Fills the Screenbuffer with values from a given buffer of a fixed length */
SSD1306_Error_t ssd1306_FillBuffer(uint8_t* buf, uint32_t len)
{
    SSD1306_Error_t ret = SSD1306_ERR;
    if (len <= SSD1306_BUFFER_SIZE) {
        memcpy_s(SSD1306_Buffer, sizeof(SSD1306_Buffer), buf, len);
        ret = SSD1306_OK;
    }
    return ret;
}

// Initialize the oled screen
void ssd1306_Init(void)
{
    // Reset OLED
    ssd1306_Reset();

    // Wait for the screen to boot
    unsigned int time = 100;
    HAL_Delay(time);

    // Init OLED
    ssd1306_SetDisplayOn(0); // display off

    ssd1306_WriteCommand(0x20); // Set Memory Addressing Mode
    ssd1306_WriteCommand(0x00); // 00b, Horizontal Addressing Mode; 01b, Vertical Addressing Mode;
                                // 10b, Page Addressing Mode (RESET); 11b, Invalid

    ssd1306_WriteCommand(0xB0); // Set Page Start Address for Page Addressing Mode, 0-7

#ifdef SSD1306_MIRROR_VERT
    ssd1306_WriteCommand(0xC0); // Mirror vertically
#else
    ssd1306_WriteCommand(0xC8); // Set COM Output Scan Direction
#endif

    ssd1306_WriteCommand(0x00); // ---set low column address
    ssd1306_WriteCommand(0x10); // ---set high column address

    ssd1306_WriteCommand(0x40); // --set start line address - CHECK

    ssd1306_SetContrast(0xFF);

#ifdef SSD1306_MIRROR_HORIZ
    ssd1306_WriteCommand(0xA0); // Mirror horizontally
#else
    ssd1306_WriteCommand(0xA1); // --set segment re-map 0 to 127 - CHECK
#endif

#ifdef SSD1306_INVERSE_COLOR
    ssd1306_WriteCommand(0xA7); // --set inverse color
#else
    ssd1306_WriteCommand(0xA6); // --set normal color
#endif

// Set multiplex ratio.
#if (SSD1306_HEIGHT == 128)
    // Found in the Luma Python lib for SH1106.
    ssd1306_WriteCommand(0xFF);
#else
    ssd1306_WriteCommand(0xA8); // --set multiplex ratio(1 to 64)- CHECK
#endif

#if (SSD1306_HEIGHT == 32)
    ssd1306_WriteCommand(0x1F);
#elif (SSD1306_HEIGHT == 64)
    ssd1306_WriteCommand(0x3F);
#elif (SSD1306_HEIGHT == 128)
    ssd1306_WriteCommand(0x3F); // Seems to work for 128px high displays too.
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

    ssd1306_WriteCommand(0xA4); // 0xa4, Output follows RAM content;0xa5, Output ignores RAM content

    ssd1306_WriteCommand(0xD3); // -set display offset - CHECK
    ssd1306_WriteCommand(0x00); // -not offset

    ssd1306_WriteCommand(0xD5); // --set display clock divide ratio/oscillator frequency
    ssd1306_WriteCommand(0xF0); // --set divide ratio

    ssd1306_WriteCommand(0xD9); // --set pre-charge period
    ssd1306_WriteCommand(0x11); // 0x22 by default

    ssd1306_WriteCommand(0xDA); // --set com pins hardware configuration - CHECK
#if (SSD1306_HEIGHT == 32)
    ssd1306_WriteCommand(0x02);
#elif (SSD1306_HEIGHT == 64)
    ssd1306_WriteCommand(0x12);
#elif (SSD1306_HEIGHT == 128)
    ssd1306_WriteCommand(0x12);
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

    ssd1306_WriteCommand(0xDB); // --set vcomh
    ssd1306_WriteCommand(0x30); // 0x20, 0.77xVcc, 0x30, 0.83xVcc

    ssd1306_WriteCommand(0x8D); // --set DC-DC enable
    ssd1306_WriteCommand(0x14);
    ssd1306_SetDisplayOn(1); // --turn on SSD1306 panel

    // Clear screen
    ssd1306_Fill(Black);

    // Flush buffer to screen
    ssd1306_UpdateScreen();

    // Set default values for screen object
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;

    SSD1306.Initialized = 1;
}

// Fill the whole screen with the given color
void ssd1306_Fill(SSD1306_COLOR color)
{
    /* Set memory */
    uint32_t i;

    for (i = 0; i < sizeof(SSD1306_Buffer); i++) {
        SSD1306_Buffer[i] = (color == Black)? 0x00 : 0xFF;
    }
}

// Write the screenbuffer with changed to the screen
void ssd1306_UpdateScreen(void)
{
    // Write data to each page of RAM. Number of pages
    // depends on the screen height:
    // * 32px   ==  4 pages
    // * 64px   ==  8 pages
    // * 128px  ==  16 pages

    uint8_t cmd[] = {
        0X21,   // 设置列起始和结束地址
        0X00,   // 列起始地址 0
        0X7F,   // 列终止地址 127
        0X22,   // 设置页起始和结束地址
        0X00,   // 页起始地址 0
        0X07,   // 页终止地址 7
    };
    uint32_t count = 0;
    uint8_t data[sizeof(cmd)*DOUBLE + SSD1306_BUFFER_SIZE + 1] = {};

    // copy cmd
    for (uint32_t i = 0; i < sizeof(cmd)/sizeof(cmd[0]); i++) {
        data[count++] = SSD1306_CTRL_CMD | SSD1306_MASK_CONT;
        data[count++] = cmd[i];
    }

    // copy frame data
    data[count++] = SSD1306_CTRL_DATA;
    memcpy_s(&data[count], sizeof(data[count]), SSD1306_Buffer, sizeof(SSD1306_Buffer));
    count += sizeof(SSD1306_Buffer);

    // send to i2c bus
    uint32_t retval = ssd1306_SendData(data, count);
    if (retval != IOT_SUCCESS) {
        printf("ssd1306_UpdateScreen send frame data filed: %ud!\r\n", retval);
    }
}

// Draw one pixel in the screenbuffer
// X => X Coordinate
// Y => Y Coordinate
// color => Pixel color
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        // Don't write outside the buffer
        return;
    }

    SSD1306_COLOR color_temp = color;
    // Check if pixel should be inverted
    if (SSD1306.Inverted) {
        color_temp = (SSD1306_COLOR)!color_temp;
    }

    // Draw in the right color
    uint32_t c = 8;
    if (color_temp == White) {
        SSD1306_Buffer[x + (y / c)* SSD1306_WIDTH] |= 1 << (y % c);
    } else {
        SSD1306_Buffer[x + (y / c)* SSD1306_WIDTH] &= ~(1 << (y % c));
    }
}

// Draw 1 char to the screen buffer
// ch       => char om weg te schrijven
// Font     => Font waarmee we gaan schrijven
// color    => Black or White
char ssd1306_DrawChar(char ch, FontDef Font, SSD1306_COLOR color)
{
    uint32_t i, b, j;

    // Check if character is valid
    uint32_t ch_min = 32;
    uint32_t ch_max = 126;
    if (ch < ch_min || ch > ch_max) {
        return 0;
    }
    // Check remaining space on current line
    if (SSD1306_WIDTH < (SSD1306.CurrentX + Font.FontWidth)||
        SSD1306_HEIGHT < (SSD1306.CurrentY + Font.FontHeight)) {
        // Not enough space on current line
        return 0;
    }

    // Use the font to write
    for (i = 0; i < Font.FontHeight; i++) {
        b = Font.data[(ch - ch_min)* Font.FontHeight + i];
        for (j = 0; j < Font.FontWidth; j++) {
            if ((b << j)& 0x8000) {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR)color);
            } else {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR)!color);
            }
        }
    }

    // The current space is now taken
    SSD1306.CurrentX += Font.FontWidth;

    // Return written char for validation
    return ch;
}

// Write full string to screenbuffer
char ssd1306_DrawString(char* str, FontDef Font, SSD1306_COLOR color)
{
    // Write until null-byte
    char* str_temp = str;
    while (*str_temp) {
        if (ssd1306_DrawChar(*str_temp, Font, color)!= *str_temp) {
            // Char could not be written
            return *str_temp;
        }

        // Next char
        str_temp++;
    }

    // Everything ok
    return *str_temp;
}

// Position the cursor
void ssd1306_SetCursor(uint8_t x, uint8_t y)
{
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}

// Draw line by Bresenhem's algorithm
void ssd1306_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_COLOR color)
{
    int32_t deltaX = abs(x2 - x1);
    int32_t deltaY = abs(y2 - y1);
    int32_t signX = ((x1 < x2)? 1 : -1);
    int32_t signY = ((y1 < y2)? 1 : -1);
    int32_t error = deltaX - deltaY;
    int32_t error2;
    uint8_t y1_temp = y1;
    uint8_t x1_temp = x1;

    ssd1306_DrawPixel(x2, y2, color);
    while ((x1_temp != x2)|| (y1_temp != y2)) {
        ssd1306_DrawPixel(x1_temp, y1_temp, color);
        error2 = error * DOUBLE;
    if (error2 > -deltaY) {
        error -= deltaY;
        x1_temp += signX;
    } else {
    /* nothing to do */
    }

    if (error2 < deltaX) {
        error += deltaX;
        y1_temp += signY;
    } else {
    /* nothing to do */
    }
    }
    return;
}
// Draw polyline
void ssd1306_DrawPolyline(const SSD1306_VERTEX *par_vertex, uint16_t par_size, SSD1306_COLOR color)
{
    uint16_t i;
    if (par_vertex != 0) {
        for (i = 1; i < par_size; i++) {
        ssd1306_DrawLine(par_vertex[i - 1].x, par_vertex[i - 1].y, par_vertex[i].x, par_vertex[i].y, color);
        }
    } else {
    /* nothing to do */
    }
    return;
}
/* Convert Degrees to Radians */
static float ssd1306_DegToRad(float par_deg)
{
    float pi = 3.14;
    float angle = 180;
    return par_deg * pi / angle;
}
/* Normalize degree to [0;360] */
static uint16_t ssd1306_NormalizeTo0_360(uint16_t par_deg)
{
    unsigned int angle = 360;
    uint16_t loc_angle;
    if (par_deg <= angle) {
        loc_angle = par_deg;
    } else {
        loc_angle = par_deg % angle;
        loc_angle = ((par_deg != 0)?par_deg:angle);
    }
    return loc_angle;
}
/* DrawArc. Draw angle is beginning from 4 quart of trigonometric circle (3pi/2)
 * start_angle in degree
 * sweep in degree
 */
void ssd1306_DrawArc(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, SSD1306_COLOR color)
{
#define CIRCLE_APPROXIMATION_SEGMENTS 36
    float approx_degree;
    uint32_t approx_segments;
    uint8_t xp1, xp2;
    uint8_t yp1, yp2;
    uint32_t count = 0;
    uint32_t loc_sweep = 0;
    float rad;
    unsigned int angle = 360;

    loc_sweep = ssd1306_NormalizeTo0_360(sweep);

    count = (ssd1306_NormalizeTo0_360(start_angle)* CIRCLE_APPROXIMATION_SEGMENTS)/ angle;
    approx_segments = (loc_sweep * CIRCLE_APPROXIMATION_SEGMENTS)/ angle;
    approx_degree = loc_sweep / (float)approx_segments;
    while (count < approx_segments) {
        rad = ssd1306_DegToRad(count*approx_degree);
        xp1 = x + (int8_t)(sin(rad)*radius);
        yp1 = y + (int8_t)(cos(rad)*radius);
        count++;
        if (count != approx_segments) {
            rad = ssd1306_DegToRad(count*approx_degree);
        } else {
            rad = ssd1306_DegToRad(loc_sweep);
        }
        xp2 = x + (int8_t)(sin(rad)*radius);
        yp2 = y + (int8_t)(cos(rad)*radius);
        ssd1306_DrawLine(xp1, yp1, xp2, yp2, color);
    }

    return;
}
// Draw circle by Bresenhem's algorithm
void ssd1306_DrawCircle(uint8_t par_x, uint8_t par_y, uint8_t par_r, SSD1306_COLOR par_color)
{
    int32_t x = -par_r;
    int32_t y = 0;
    int32_t b = 2;
    int32_t err = b - b * par_r;
    int32_t e2;

    if (par_x >= SSD1306_WIDTH || par_y >= SSD1306_HEIGHT) {
        return;
    }

    do {
        ssd1306_DrawPixel(par_x - x, par_y + y, par_color);
        ssd1306_DrawPixel(par_x + x, par_y + y, par_color);
        ssd1306_DrawPixel(par_x + x, par_y - y, par_color);
        ssd1306_DrawPixel(par_x - x, par_y - y, par_color);
        e2 = err;
        if (e2 <= y) {
            y++;
            err = err + (y * b + 1);
            if (-x == y && e2 <= x) {
                e2 = 0;
            } else {
                /* nothing to do */
            }
        } else {
            /* nothing to do */
        }
        if (e2 > x) {
            x++;
            err = err + (x * b + 1);
        } else {
            /* nothing to do */
        }
    } while (x <= 0);

    return;
}

// Draw rectangle
void ssd1306_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_COLOR color)
{
    ssd1306_DrawLine(x1, y1, x2, y1, color);
    ssd1306_DrawLine(x2, y1, x2, y2, color);
    ssd1306_DrawLine(x2, y2, x1, y2, color);
    ssd1306_DrawLine(x1, y2, x1, y1, color);

    return;
}

void ssd1306_DrawBitmap(const uint8_t* bitmap, uint32_t size)
{
    unsigned int c = 8;
    uint8_t rows = size * c / SSD1306_WIDTH;
    if (rows > SSD1306_HEIGHT) {
        rows = SSD1306_HEIGHT;
    }
    for (uint8_t y = 0; y < rows; y++) {
        for (uint8_t x = 0; x < SSD1306_WIDTH; x++) {
            uint8_t byte = bitmap[(y * SSD1306_WIDTH / c)+ (x / c)];
            uint8_t bit = byte & (0x80 >> (x % c));
            ssd1306_DrawPixel(x, y, bit ? White : Black);
        }
    }
}

void ssd1306_DrawRegion(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* data, uint32_t size, uint32_t stride)
{
    if (x + w > SSD1306_WIDTH || y + h > SSD1306_HEIGHT || w * h == 0) {
        printf("%dx%d @ %d, %d out of range or invalid!\r\n", w, h, x, y);
        return;
    }

    uint8_t w_temp = w;
    uint8_t h_temp = h;
    uint32_t stride_temp = stride;
    w_temp = (w_temp <= SSD1306_WIDTH ? w_temp : SSD1306_WIDTH);
    h_temp = (h_temp <= SSD1306_HEIGHT ? h_temp : SSD1306_HEIGHT);
    stride_temp = (stride_temp == 0 ? w_temp : stride_temp);
    unsigned int c = 8;

    uint8_t rows = size * c / stride_temp;
    for (uint8_t i = 0; i < rows; i++) {
        uint32_t base = i * stride_temp / c;
        for (uint8_t j = 0; j < w_temp; j++) {
            uint32_t idx = base + (j / c);
            uint8_t byte = idx < size ? data[idx] : 0;
            uint8_t bit = byte & (0x80 >> (j % c));
            ssd1306_DrawPixel(x + j, y + i, bit ? White : Black);
        }
    }
}

void ssd1306_SetContrast(const uint8_t value)
{
    const uint8_t kSetContrastControlRegister = 0x81;
    ssd1306_WriteCommand(kSetContrastControlRegister);
    ssd1306_WriteCommand(value);
}

void ssd1306_SetDisplayOn(const uint8_t on)
{
    uint8_t value;
    if (on) {
        value = 0xAF; // Display on
        SSD1306.DisplayOn = 1;
    } else {
        value = 0xAE; // Display off
        SSD1306.DisplayOn = 0;
    }
    ssd1306_WriteCommand(value);
}

uint8_t ssd1306_GetDisplayOn(void)
{
    return SSD1306.DisplayOn;
}
