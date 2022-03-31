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

// #ifdef CONFIG_HISTREAMING_SUPPORT
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <hi_pwm.h>
#include <hi_time.h>
/* Link Header Files */
#include <link_service.h>
#include <link_platform.h>
#include <histreaming.h>
#include <hi_io.h>
#include <hi_early_debug.h>
#include <hi_gpio.h>
#include <hi_task.h>
#include <hi_types_base.h>
#include "app_demo_multi_sample.h"
#include "ssd1306_oled.h"
#include "iot_gpio.h"

#define HISTREAMING_TASK
#ifdef HISTREAMING_TASK
#define HISTREAMING_DEMO_TASK_STAK_SIZE (1024*8)
#define HISTREAMING_DEMO_TASK_PRIORITY  25
#define IO_FUNC_GPIO_OUT 0
#define IOT_GPIO_INDEX_10 10
#define IOT_GPIO_INDEX_11 11
#define IOT_GPIO_INDEX_12 12
hi_u32 g_histreamingDemoTaskId = 0;
#endif

static hi_void HistreamingTrafficLightControl(const char* property, char* value);

static void HistreamingManualControlModule(const char* property, char* value)
{
    if (strcmp(property, "tl_s") == 0) {
        if (strcmp(value, "red_on") == 0) {
            OledShowStr(OLED_X_POSITION_0, OLED_Y_POSITION_7, "1.Red On       ",
                        OLED_DISPLAY_STRING_TYPE_1);  /* 0, 7, xx, 1 */
            GpioControl(IOT_GPIO_INDEX_10, IOT_GPIO_INDEX_10,
                        IOT_GPIO_DIR_OUT, IOT_GPIO_VALUE1, IO_FUNC_GPIO_OUT);
            GpioControl(IOT_GPIO_INDEX_11, IOT_GPIO_INDEX_11,
                        IOT_GPIO_DIR_OUT, IOT_GPIO_VALUE0, IO_FUNC_GPIO_OUT);
        }
        if (strcmp(value, "yellow_on") == 0) {
            OledShowStr(OLED_X_POSITION_0, OLED_Y_POSITION_7, "2.Yellow On     ",
                        OLED_DISPLAY_STRING_TYPE_1); /* 0, 7, xx, 1 */
            GpioControl(IOT_GPIO_INDEX_10, IOT_GPIO_INDEX_10,
                        IOT_GPIO_DIR_OUT, IOT_GPIO_VALUE0, IO_FUNC_GPIO_OUT);
            GpioControl(IOT_GPIO_INDEX_12, IOT_GPIO_INDEX_12,
                        IOT_GPIO_DIR_OUT, IOT_GPIO_VALUE1, IO_FUNC_GPIO_OUT);
        }
        if (strcmp(value, "green_on") == 0) {
            OledShowStr(OLED_X_POSITION_0, OLED_Y_POSITION_7, "3.Green On      ",
                        OLED_DISPLAY_STRING_TYPE_1); /* 0, 7, xx, 1 */
            GpioControl(IOT_GPIO_INDEX_12, IOT_GPIO_INDEX_12,
                        IOT_GPIO_DIR_OUT, IOT_GPIO_VALUE0, IO_FUNC_GPIO_OUT);
            GpioControl(IOT_GPIO_INDEX_11, IOT_GPIO_INDEX_11,
                        IOT_GPIO_DIR_OUT, IOT_GPIO_VALUE1, IO_FUNC_GPIO_OUT);
        }
    }
}
static void HistreamingAutoModule(const char* property, char* value)
{
    if (strcmp(property, "tla_s") == 0) {
        if (strcmp(value, "tla") == 0) {
            int ret = SetKeyStatus(TRAFFIC_AUTO_MODE);
            if (ret != TRAFFIC_AUTO_MODE) {
                return;
            }
        }
    }
}

static void HistreamingHumanModuleNormal(const char* property, char* value)
{
    if (strcmp(property, "tlh_s") == 0) {
        if (strcmp(value, "tlh") == 0) {
            int rc = SetKeyType(TRAFFIC_NORMAL_TYPE);
            if (rc != TRAFFIC_NORMAL_TYPE) {
                return;
            }
            int ret = SetKeyStatus(TRAFFIC_HUMAN_MODE);
            if (ret != TRAFFIC_HUMAN_MODE) {
                return;
            }
        }
    }
}

static hi_u32 HistreamingTrafficLightReturnMainMenu(const char* property, char* value)
{
    if (strcmp(property, "tlr_s") == 0) {
        if (strcmp(value, "tlr") == 0) {
            int ret = SetKeyStatus(TRAFFIC_RETURN_MODE);
            if (ret != TRAFFIC_RETURN_MODE) {
                return;
            }
            return HI_ERR_SUCCESS;
        }
    }
}


/* histreaming traffic light function control */
static hi_void HistreamingTrafficLightControl(const char* property, char* value)
{
    /* manual control module */
    HistreamingManualControlModule(property, value);
    /* auto module */
    HistreamingAutoModule(property, value);
    // /* human module normal */
    HistreamingHumanModuleNormal(property, value);
    HistreamingTrafficLightReturnMainMenu(property, value);
}

static int GetStatusValue(const struct LinkService* ar, const char* property, const char* value, int len)
{
    (void)(ar);
    char *status = "Opend";

    printf("Receive property: %s(value=%s[%d])\n", property, value, len);

    if (strcmp(property, "Status") == 0) {
        (void)strcpy_s(value, strlen(status) + 1, "Opend"); /* 1: '\0' lenght */
    }
    /*
     * if Ok return 0,
     * Otherwise, any error, return StatusFailure
     */
    return 0;
}
/* recv from app cmd */
static int ModifyStatus(const struct LinkService* ar, const char* property, const char* value, int len)
{
    int ret;
    (void)(ar);
    printf("-----------$$$$$$$$$$$$\n");

    if (property == NULL || value == NULL) {
        return -1;
    }
    /* modify status property */
    /* traffic light module */
    HistreamingTrafficLightControl(property, value);
    /* traffic light return main menu */
    ret = HistreamingTrafficLightReturnMainMenu(property, value);
    if (ret != 0) {
        printf("HistreamingTrafficLightReturnMainMenu Failed\r\n");
    }
    printf("Receive property: %s(value=%s[%d])\n", property, value, len);
    /*
     * if Ok return 0,
     * Otherwise, any error, return StatusFailure
     */
    return 0;
}

/*
 * It is a Wifi IoT device
 */
static const char* g_wifiStaType = "Light";
static const char* GetDeviceType(const struct LinkService* ar)
{
    (void)(ar);

    return g_wifiStaType;
}

static void *g_linkPlatform = NULL;

void* HistreamingOpen(void)
{
    LinkService* wifiIot = 0;
    LinkPlatform* link = 0;

    wifiIot = (LinkService*)malloc(sizeof(LinkService));
    if (!wifiIot) {
        printf("malloc wifiIot failure\n");
        return NULL;
    }

    wifiIot->get    = GetStatusValue;
    wifiIot->modify = ModifyStatus;
    wifiIot->type = GetDeviceType;
    link = LinkPlatformGet();
    if (!link) {
        printf("get link failure\n");
    }

    if (link->addLinkService(link, wifiIot, 1) != 0) {
        HistreamingClose(link);
        return NULL;
    }
    if (link->open(link) != 0) {
        HistreamingClose(link);
        return NULL;
    }
    /* cache link ptr */
    g_linkPlatform = (void*)(link);
#ifdef HISTREAMING_TASK
    hi_task_delete(g_histreamingDemoTaskId);
#endif
    hi_free(0, wifiIot);
    return (void*)link;
}

void HistreamingClose(const char *link)
{
    LinkPlatform *linkPlatform = (LinkPlatform*)(link);
    if (!linkPlatform) {
        return;
    }

    linkPlatform->close(linkPlatform);

    if (linkPlatform != NULL) {
        LinkPlatformFree(linkPlatform);
    }
}
#ifdef HISTREAMING_TASK
hi_void HistreamingDemo(hi_void)
{
    hi_u32 ret;
    hi_task_attr histreaming = {0};
    histreaming.stack_size = HISTREAMING_DEMO_TASK_STAK_SIZE;
    histreaming.task_prio = HISTREAMING_DEMO_TASK_PRIORITY;
    histreaming.task_name = "histreaming_demo";
    ret = hi_task_create(&g_histreamingDemoTaskId, &histreaming, HistreamingOpen, HI_NULL);
    if (ret != HI_ERR_SUCCESS) {
        printf("Falied to create histreaming demo task!\n");
    }
}
#endif
// #endif