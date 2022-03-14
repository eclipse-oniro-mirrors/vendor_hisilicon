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

#include <string.h>
#include <hi_wifi_api.h>
#include <hi_mux.h>
#include <hi_task.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_config.h"
#include "iot_log.h"
#include "iot_main.h"
#include "iot_profile.h"

/* attribute initiative to report */
#define TAKE_THE_INITIATIVE_TO_REPORT
/* oc request id */
#define CN_COMMADN_INDEX    "commands/request_id="

// this is the callback function, set to the mqtt, and if any messages come, it will be called
// The payload here is the json string
static void DemoMsgRcvCallBack(int qos, const char *topic, const char *payload)
{
    const char *requesID;
    char *tmp;
    IoTCmdRespT resp;
    IOT_LOG_DEBUG("RCVMSG:QOS:%d TOPIC:%s PAYLOAD:%s\r\n", qos, topic, payload);

    tmp = strstr(topic, CN_COMMADN_INDEX);
    if (tmp != NULL) {
        // <now you could deal your own works here --THE COMMAND FROM THE PLATFORM
        // <now er roport the command execute result to the platform
        requesID = tmp + strlen(CN_COMMADN_INDEX);
        resp.requestID = requesID;
        resp.respName = NULL;
        resp.retCode = 0;  // <which means 0 success and others failed
        resp.paras = NULL;
        (void)IoTProfileCmdResp(CONFIG_DEVICE_PWD, &resp);
    }
    return;
}

/* Smart Can */
hi_void IotPublishPersionTime(hi_u32 time)
{
    IoTProfileServiceT service;
    IoTProfileKVT property;

    memset_s(&property, sizeof(property), 0, sizeof(property));
    property.type = EN_IOT_DATATYPE_INT;
    property.key = "Person_times";

    property.iValue = time;

    memset_s(&service, sizeof(service), 0, sizeof(service));
    service.serviceID = "helloMQTT";
    service.serviceProperty = &property;

    IoTProfilePropertyReport(CONFIG_DEVICE_ID, &service);
}

// this is the demo main task entry,here we will set the wifi/cjson/mqtt ready ,and
// wait if any work to do in the while
static hi_void *DemoEntry(hi_void *arg)
{
    WifiStaReadyWait();
}

// This is the demo entry, we create a task here, and all the works has been done in the demo_entry
#define CN_IOT_TASK_STACKSIZE 0x1000
#define CN_IOT_TASK_PRIOR 25
#define CN_IOT_TASK_NAME "IOTDEMO"

static void AppDemoIot(void)
{
    osThreadAttr_t attr;
    IoTWatchDogDisable();

    attr.name = "IOTDEMO";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = CN_IOT_TASK_STACKSIZE;
    attr.priority = CN_IOT_TASK_PRIOR;

    if (osThreadNew((osThreadFunc_t)DemoEntry, NULL, &attr) == NULL) {
        printf("[TrafficLight] Falied to create IOTDEMO!\n");
    }
}

SYS_RUN(AppDemoIot);