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

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <hi_pwm.h>
#include <hi_time.h>
/* Link Header Files */
#include <link_service.h>
#include <link_platform.h>
#include <hi_io.h>
#include <hi_early_debug.h>
#include <hi_gpio.h>
#include <hi_types_base.h>
#include <hi_stdlib.h>
#include <hi_task.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio_ex.h"
#include "wifi_connecter.h"
#include "iot_gpio.h"
#include "app_demo_histreaming.h"

#define LED_TEST_GPIO 9
#define LED_INTERVAL_TIME_US 300000

#define HISTREAMING_DEMO_TASK_STAK_SIZE (1024*8)
#define HISTREAMING_DEMO_TASK_PRIORITY  25
#define REV_BUFF_LEN    512

char rev_buff[512] = {0};
unsigned char hex_buff[512] = {0};
unsigned int hex_len = 0;

UartDefConfig uartDefConfig = {0};

int StringToHex(char *str, unsigned char *out, unsigned int *outlen)
{
    char *p = str;
    char high = 0, low = 0;
    int tmplen = strlen(p), cnt = 0;
    tmplen = strlen(p);
    while (cnt < (tmplen / HIGH_NUM)) {
        high = ((*p > HIGH_ASCII) && ((*p <= 'F') || (*p <= 'f'))) ? *p - HIGH_NUM2 - HIGH_NUM3 : *p - HIGH_NUM2;
        low = (*(++ p) > HIGH_ASCII && ((*p <= 'F') || (*p <= 'f'))) ? *(p) - HIGH_NUM2 - HIGH_NUM3 : *(p) - HIGH_NUM2;
        out[cnt] = (((high & 0x0f) << HIGH_NUM4) | (low & 0x0f));
        p ++;
        cnt ++;
    }
    if (tmplen % HIGH_NUM != 0) {
        out[cnt] = ((*p > HIGH_ASCII) && ((*p <= 'F') || (*p <= 'f'))) ? *p - HIGH_NUM2 - HIGH_NUM3 : *p - HIGH_NUM2;
    }
    if (outlen != NULL) {
        *outlen = tmplen / HIGH_NUM + tmplen % HIGH_NUM;
    }
    return tmplen / HIGH_NUM + tmplen % HIGH_NUM;
}

/**
 * @bref The device side sends the characteristic value to the app side
 * @param struct LinkService* ar: histreaming LinkServer structural morphology
 * @param const char* property: characteristic value
 * @param char* value: send value to apps
 * @param int len: send value length
 */
static int GetStatusValue(struct LinkService* ar, const char* property, char* value, int len)
{
    (void)(ar);
    if (strcmp(property, "Status") == 0) {
    }
/*
 * if Ok return 0,
 * Otherwise, any error, return StatusFailure
 */
    return 0;
}
/**
 * @bref recv from app cmd
 * @bref Receive the message sent by the app, and operate the hi3861 device side accordingly
 * @param struct LinkService* ar: histreaming LinkServer structural morphology
 * @param const char* property: Eigenvalues sent by app
 * @param char* value: Value sent by app
 * @param int len: Length of APP sent
 */
static int ModifyStatus(struct LinkService* ar, const char* property, char* value, int len)
{
    (void)(ar);
    printf("Receive property: %s(value=%s, [%d])\n", property, value, len);
    if (property == NULL || value == NULL) {
        return -1;
    }
    if (memcpy_s(rev_buff, REV_BUFF_LEN, value, len) != 0) {
        return 0;
    }
    /* modify status property */
    /* colorful light module */
    printf("%s, %d\r\n",  rev_buff, len);

    StringToHex(rev_buff, hex_buff, &hex_len);
    for (int i = 0; i < hex_len; i++) {
        printf("0x%x ", hex_buff[i]);
    }
    printf("\r\n");
    IoTGpioSetOutputVal(LED_TEST_GPIO, 0);
    usleep(LED_INTERVAL_TIME_US);
    IoTGpioSetOutputVal(LED_TEST_GPIO, 1);
    usleep(LED_INTERVAL_TIME_US);
/*
 * if Ok return 0,
 * Otherwise, any error, return StatusFailure
 */
    return 0;
}

/*
 * It is a Wifi IoT device
 */
static const char* g_wifiStaType = "Pegasus:Hi3861";
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

hi_void HistreamingDemo(hi_void)
{
    ConnectToHotspot();
    IoTGpioInit(LED_TEST_GPIO);
    IoTGpioSetDir(LED_TEST_GPIO, IOT_GPIO_DIR_OUT);
    osThreadAttr_t histreaming = {0};
    histreaming.stack_size = HISTREAMING_DEMO_TASK_STAK_SIZE;
    histreaming.priority = HISTREAMING_DEMO_TASK_PRIORITY;
    histreaming.name = (hi_char*)"histreaming_demo";
    if (osThreadNew((osThreadFunc_t)HistreamingOpen, NULL, &histreaming) == NULL) {
        printf("Failed to create histreaming task\r\n");
    }
}

SYS_RUN(HistreamingDemo);