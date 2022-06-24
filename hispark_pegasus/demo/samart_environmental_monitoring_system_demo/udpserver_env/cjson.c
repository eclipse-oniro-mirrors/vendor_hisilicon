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

#include <stdint.h>
#include <cJSON.h>
#include <stdlib.h>
#include <hi_mem.h>
#include "config_params.h"
#include "wifi_device_config.h"
#include "kv_store.h"
#include "reset.h"
#include "stdbool.h"
#include "cmsis_os2.h"
#include "ohos_init.h"
#define ATTR.STACK_SIZE 4096
#define TWO_HUNDRED 200

#define SYS_REBOOT_CAUSE_USR_NORMAL_REBOOT 5

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

char *cJSONReport(void)
{
    cJSON* cjson_test = NULL;
    cJSON* cjson_temp = NULL;
    cJSON* cjson_humi = NULL;
    cJSON* cjson_gas = NULL;
    char* str = NULL;

    /* 创建一个JSON数据对象(链表头结点) */
    cjson_test = cJSON_CreateObject();

    /* 添加一条浮点类型的JSON数据(添加一个链表节点) */
    cJSON_AddNumberToObject(cjson_test, "temp", g_temperature);
    cJSON_AddNumberToObject(cjson_test, "humi", g_humidity);
    cJSON_AddNumberToObject(cjson_test, "gas", g_gasValuetemp);

    str = cJSON_Print(cjson_test);
    printf("%s\n", str);
    return str;
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
    cJSON* cjson_temp_max = NULL;
    cJSON* cjson_temp_min = NULL;
    cJSON* cjson_humi_max = NULL;
    cJSON* cjson_humi_min = NULL;
    cJSON* cjson_gas_value = NULL;
    cJSON* cjson_update = NULL;

    cjson_test_sta = cJSON_Parse(message);
    if (cjson_test_sta == NULL) {
        printf("parse fail.\n");
        return -1;
    }

    /* 依次根据名称提取JSON数据（键值对） */
    cjson_temp_max = cJSON_GetObjectItem(cjson_test_sta, "temp_max");
    cjson_temp_min = cJSON_GetObjectItem(cjson_test_sta, "temp_min");
    cjson_humi_max = cJSON_GetObjectItem(cjson_test_sta, "humi_max");
    cjson_humi_min = cJSON_GetObjectItem(cjson_test_sta, "humi_min");
    cjson_gas_value = cJSON_GetObjectItem(cjson_test_sta, "gas_value");
    cjson_update = cJSON_GetObjectItem(cjson_test_sta, "update");

    int ret = 0;
    if (cjson_temp_max != NULL) {
        ret = UtilsSetValue(PARAM_TEMP_MAX, cjson_temp_max->valuestring);
        if (ret < 0) {
            printf("temp_max value saved filed, ret is %d\r\n", ret);
        }
    }
    if (cjson_temp_min != NULL) {
        ret = UtilsSetValue(PARAM_TEMP_MIN, cjson_temp_min->valuestring);
        if (ret < 0) {
            printf("temp_min value saved filed, ret is %d\r\n", ret);
        }
    }
    if (cjson_humi_max != NULL) {
        ret += UtilsSetValue(PARAM_HUMI_MAX, cjson_humi_max->valuestring);
        if (ret < 0) {
            printf("humi_max value saved filed, ret is %d\r\n", ret);
        }
    }
    if (cjson_humi_min != NULL) {
        ret = UtilsSetValue(PARAM_HUMI_MIN, cjson_humi_min->valuestring);
        if (ret < 0) {
            printf("humi_min value saved filed, ret is %d\r\n", ret);
        }
    }
    if (cjson_update != NULL) {
        sendMessage = 1;
    }
    cJSON_Delete(cjson_test_sta);

    return ret;
}
