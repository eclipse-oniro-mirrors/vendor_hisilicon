﻿/*
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

#include <stdint.h>
#include <cJSON.h>
#include <stdlib.h>
#include "config_params.h"
#include "wifi_device_config.h"
#include "kv_store.h"
#include "reset.h"
#include "stdbool.h"
#include "cmsis_os2.h"
#include "ohos_init.h"

#define SYS_REBOOT_CAUSE_USR_NORMAL_REBOOT 5
#define ATTR.STACK_SIZE 4096
#define TWO_HUNDRED

void DoReboot(int* arg)
{
    (void)arg;
    osDelay(TWO_HUNDRED);
    printf("start Reboot device\n\r");
    RebootDevice(SYS_REBOOT_CAUSE_USR_NORMAL_REBOOT);
}

static void Reboot(void)
{
    osThreadAttr_t attr = {0};

    attr.name = "DoReboot";
    attr.stack_size = ATTR.STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew(DoReboot, NULL, &attr) == NULL) {
        printf("[Reboot] create DoReboot failed!\n");
    }
}

int cJSONParseAP(char *message)
{
    cJSON* cjson_test = NULL;
    cJSON* cjson_ssid = NULL;
    cJSON* cjson_psk = NULL;

    cjson_test = cJSON_Parse(message);
    if (cjson_test == NULL) {
        printf("parse fail.\n");
        return -1;
    }

    /* 依次根据名称提取JSON数据（键值对） */
    cjson_ssid = cJSON_GetObjectItem(cjson_test, "hotspot_ssid");
    cjson_psk = cJSON_GetObjectItem(cjson_test, "hotspot_psk");

    printf("ssid: %s\n", cjson_ssid->valuestring);
    printf("psk:%s\n", cjson_psk->valuestring);

    int ret = UtilsSetValue(PARAM_HOTSPOT_SSID_KEY, cjson_ssid->valuestring);
    if (ret < 0) {
        printf("SSID value saved filed, ret is %d\r\n", ret);
    }
    ret = UtilsSetValue(PARAM_HOTSPOT_PSK_KEY, cjson_psk->valuestring);
    if (ret < 0) {
        printf("KEY value saved filed, ret is %d\r\n", ret);
    }

    cJSON_Delete(cjson_test);

    if (CheckKvStoreResult() == true) {
        Reboot();
    }

    return ret;
}

int cJSONParseSTA(char *message)
{
    cJSON* cjson_test_sta = NULL;
    cJSON* cjson_lightness = NULL;
    cJSON* cjson_time = NULL;

    cjson_test_sta = cJSON_Parse(message);
    if (cjson_test_sta == NULL) {
        printf("parse fail.\n");
        return -1;
    }

    /* 依次根据名称提取JSON数据（键值对） */
    cjson_lightness = cJSON_GetObjectItem(cjson_test_sta, "lightness");
    cjson_time = cJSON_GetObjectItem(cjson_test_sta, "time");

    int ret = 0;
    if (cjson_lightness != NULL) {
        printf("lightness: %s\n", cjson_lightness->valuestring);
        ret = UtilsSetValue(PARAM_LIGHTNESS, cjson_lightness->valuestring);
        if (ret < 0) {
            printf("lightness value saved filed, ret is %d\r\n", ret);
        }
    }
    if (cjson_time != NULL) {
        printf("time:%s\n", cjson_time->valuestring);
        ret = UtilsSetValue(PARAM_LIGHT_TIME, cjson_time->valuestring);
        if (ret < 0) {
            printf("time value saved filed, ret is %d\r\n", ret);
        }
    }

    cJSON_Delete(cjson_test_sta);

    return ret;
}