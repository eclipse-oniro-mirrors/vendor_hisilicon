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
#include <string.h>
#include <stdio.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_i2c.h"
#include "iot_errno.h"

#include "ssd1306.h"
#include "night_light_param.h"
#include "config_params.h"
#define SEVENTY_FIVE 75
#define FIVE 5
#define TWENTY 20
#define TWENTY_FIVE 25
#define THIRTEEN 13
#define FOURTEEN 14
#define ONE_HUNDRED 100
#define TEN 10
#define SIX 6
#define FOURTY 40
#define ATTR.STACK_SIZE 4096

#define     OLED_I2C_BAUDRATE   (400*1000)
#define     IOT_I2C_IDX_0          0
// extern unsigned int g_lightStatus;
// extern unsigned int g_lightness;
// extern unsigned int g_ledDelayTime;
// extern unsigned int g_statusChanged;
unsigned short g_wifiStatus = WIFI_AP;

void OledUpdata(void)
{
    ssd1306_Fill(Black);
        
    ssd1306_SetCursor(TEN, 0);
    ssd1306_DrawString("Night Lignt", Font_11x18, White);

    ssd1306_SetCursor(TEN, TWENTY_FIVE);
    ssd1306_DrawString("Lightness:", Font_6x8, White);

    ssd1306_SetCursor(TEN, FOURTY);
    ssd1306_DrawString("DelayTime:", Font_6x8, White);
        
    if (g_lightness <= LIGHTNESS_WEAK_DUTY) {
        ssd1306_SetCursor(SEVENTY_FIVE, TWENTY_FIVE);
        ssd1306_DrawString("weak", Font_6x8, White);
    } else if (g_lightness <= LIGHTNESS_MID_DUTY && g_lightness > LIGHTNESS_WEAK_DUTY) {
        ssd1306_SetCursor(SEVENTY_FIVE, TWENTY_FIVE);
        ssd1306_DrawString("mid", Font_6x8, White);
    } else {
        ssd1306_SetCursor(SEVENTY_FIVE, TWENTY_FIVE);
        ssd1306_DrawString("high", Font_6x8, White);
    }

    if (g_ledDelayTime <= LIGHTDELAYSHORT) {
        ssd1306_SetCursor(SEVENTY_FIVE, FOURTY);
        ssd1306_DrawString("short", Font_6x8, White);
    } else if (g_ledDelayTime <= LIGHTDELAYMID && g_ledDelayTime > LIGHTDELAYSHORT) {
        ssd1306_SetCursor(SEVENTY_FIVE, FOURTY);
        ssd1306_DrawString("mid", Font_6x8, White);
    } else {
        ssd1306_SetCursor(SEVENTY_FIVE, FOURTY);
        ssd1306_DrawString("long", Font_6x8, White);
    }
    ssd1306_UpdateScreen();
    g_statusChanged = 0;
}

void OledDisplay(void)
{
    ssd1306_Fill(Black);
    ssd1306_SetCursor(FIVE, 0);
    ssd1306_DrawString("OpenHarmony", Font_11x18, White);
    ssd1306_SetCursor(TWENTY, TWENTY_FIVE);
    ssd1306_DrawString("Lightness", Font_7x10, White);

    ssd1306_SetCursor(TWENTY, FOURTY);
    ssd1306_DrawString("DelayTime", Font_7x10, White);

    ssd1306_UpdateScreen();
}

static void OledTask(int *arg)
{
    (void)arg;
    int times = 0;
    unsigned int retval = 0;

    retval = IoTGpioInit(THIRTEEN);
    if (retval != IOT_SUCCESS) {
        return retval;
    }
    retval = IoTGpioInit(FOURTEEN);
    if (retval != IOT_SUCCESS) {
        return retval;
    }
    retval = hi_io_set_func(THIRTEEN, SIX);
    if (retval != IOT_SUCCESS) {
        return retval;
    }
    retval = hi_io_set_func(FOURTEEN, SIX);
    if (retval != IOT_SUCCESS) {
        return retval;
    }

    int ret = IoTI2cInit(IOT_I2C_IDX_0, OLED_I2C_BAUDRATE);
    ssd1306_Init();
    OledDisplay();
    osDelay(ONE_HUNDRED);
        
    while (1) {
        osDelay(TEN);
        if (g_wifiStatus == WIFI_STA) {
            int rets = GetValue();
            if (rets < 0) {
                printf("Get time ande lightness value failed\r\n");
            }
            printf("lightness is %u, time is %u\r\n", g_lightness, g_ledDelayTime);
            OledUpdata();
        }
    }
}

static void OledDemo(void)
{
    osThreadAttr_t attr;

    attr.name = "OledTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = ATTR.STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew(OledTask, NULL, &attr) == NULL) {
        printf("[OledDemo] Failed to create OledTask!\n");
    }
}

APP_FEATURE_INIT(OledDemo);