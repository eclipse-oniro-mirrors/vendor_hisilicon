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
#include <hi_task.h>
#include <hi_wifi_api.h>
#include <hi_mux.h>
#include <hi_io.h>
#include <hi_gpio.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_config.h"
#include "iot_log.h"
#include "iot_main.h"
#include "iot_profile.h"

/* attribute initiative to report */
#define TAKE_THE_INITIATIVE_TO_REPORT
#define ONE_SECOND                          (1000)
/* oc request id */
#define CN_COMMADN_INDEX                    "commands/request_id="
#define WECHAT_SUBSCRIBE_LIGHT              "light"
#define WECHAT_SUBSCRIBE_LIGHT_ON_STATE     "1"
#define WECHAT_SUBSCRIBE_LIGHT_OFF_STATE    "0"

#define PUBULISH_TIME (1000)

int g_ligthStatus = -1;

typedef void (*HiFnMsgCallBack)(hi_gpio_value val);

typedef struct FunctionCallback {
    hi_bool  stop;
    hi_u32 conLost;
    hi_u32 queueID;
    hi_u32 iotTaskID;
    HiFnMsgCallBack msgCallBack;
}FunctionCallback;

FunctionCallback g_functinoCallback;

static void DeviceConfigInit(hi_gpio_value val)
{
    hi_io_set_func(HI_IO_NAME_GPIO_9, HI_IO_FUNC_GPIO_9_GPIO);
    hi_gpio_set_dir(HI_GPIO_IDX_9, HI_GPIO_DIR_OUT);
    hi_gpio_set_ouput_val(HI_GPIO_IDX_9, val);
}

static int  DeviceMsgCallback(HiFnMsgCallBack msgCallBack)
{
    g_functinoCallback.msgCallBack = msgCallBack;
    return 0;
}

static void WechatControlDeviceMsg(hi_gpio_value val)
{
    DeviceConfigInit(val);
}

// this is the callback function, set to the mqtt, and if any messages come, it will be called
// The payload here is the json string
static void DemoMsgRcvCallBack(int qos, const char *topic, const char *payload)
{
    IOT_LOG_DEBUG("RCVMSG:QOS:%d TOPIC:%s PAYLOAD:%s\r\n", qos, topic, payload);
    /* app 下发的操作 */
    if (strstr(payload, WECHAT_SUBSCRIBE_LIGHT) != NULL) {
        if (strstr(payload, WECHAT_SUBSCRIBE_LIGHT_OFF_STATE) != NULL) {
            WechatControlDeviceMsg(HI_GPIO_VALUE1);
            g_ligthStatus = HI_FALSE;
        } else {
            WechatControlDeviceMsg(HI_GPIO_VALUE0);
            g_ligthStatus = HI_TRUE;
        }
    }
    return HI_NULL;
}

/* publish sample */
hi_void IotPublishSample(WeChatProfile weChatProfile)
{
    /* reported attribute */
    weChatProfile.subscribeType = "type";
    weChatProfile.status.subState = "state";
    weChatProfile.status.subReport = "reported";
    weChatProfile.status.reportVersion = "version";
    weChatProfile.status.Token = "clientToken";
    /* report light */
    if (g_ligthStatus == HI_TRUE) {
        weChatProfile.reportAction.subDeviceAction1 = "light";
        weChatProfile.reportAction.action1Num = 1; /* 1: 灯的状态，1表示亮，0表示灭 */
    } else if (g_ligthStatus == HI_FALSE) {
        weChatProfile.reportAction.subDeviceAction1 = "light";
        weChatProfile.reportAction.action1Num = 0;
    } else {
        weChatProfile.reportAction.subDeviceAction1 = "light";
        weChatProfile.reportAction.action1Num = 0;
    }

    /* report motor */
    weChatProfile.reportAction.subDeviceAction2 = "motor";
    weChatProfile.reportAction.action2Num = 0;
    /* report temperature */
    weChatProfile.reportAction.subDeviceAction3 = "temperature";
    weChatProfile.reportAction.action3Num = 30; /* 30:上报温度为30℃ */
    /* report humidity */
    weChatProfile.reportAction.subDeviceAction4 = "humidity";
    weChatProfile.reportAction.action4Num = 70; /* 70:上报空气湿度为70% */
    /* report light_intensity */
    weChatProfile.reportAction.subDeviceAction5 = "light_intensity";
    weChatProfile.reportAction.action5Num = 60; /* 60:上报灯亮度为60% */

    IoTProfilePropertyReport(CONFIG_DEVICE_ID, &weChatProfile);
}

/* Smart Can */
hi_void IotPublishPersionTime(hi_u32 time)
{
    IoTProfileServiceT service;
    IoTProfileKVT property;

    memset_s(&property, sizeof(property), 0, sizeof(property));
    property.type = EN_IOT_DATATYPE_INT;
    property.key = "Person_times";

    property.i_value = time;

    memset_s(&service, sizeof(service), 0, sizeof(service));
    service.serviceID = "helloMQTT";
    service.serviceProperty = &property;
    IoTProfilePropertyReport(CONFIG_DEVICE_ID, &service);
}

// this is the demo main task entry,here we will set the wifi/cjson/mqtt ready ,and
// wait if any work to do in the while
static hi_void *DemoEntry(char *arg)
{
    WeChatProfile weChatProfile = {0};
    WifiStaReadyWait();
    CJsonInit();
    IoTMain();
    /* 云端下发 */
    IoTSetMsgCallback(DemoMsgRcvCallBack);
    /* 主动上报 */
#ifdef TAKE_THE_INITIATIVE_TO_REPORT
    while (1) {
    // here you could add your own works here--we report the data to the IoTplatform
    // now we report the data to the iot platform
    // here you could add your own works here--we report the data to the IoTplatform
    /* 用户可以在这调用发布函数进行发布，需要用户自己写调用函数 */
#ifdef HW_IOT_CLOUD
        IotPublishPersionTime(PUBULISH_TIME);
#else
        IotPublishSample(weChatProfile); // 发布例程
#endif
        hi_sleep(ONE_SECOND);
    }
#endif
    return NULL;
}
// This is the demo entry, we create a task here, and all the works has been done in the demo_entry
#define CN_IOT_TASK_STACKSIZE  0x1000
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
        printf("[mqtt] Falied to create IOTDEMO!\n");
    }
}

SYS_RUN(AppDemoIot);