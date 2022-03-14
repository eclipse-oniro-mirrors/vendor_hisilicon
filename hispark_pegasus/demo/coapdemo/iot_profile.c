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
#include <hi_mem.h>
#include <cJSON.h>
#include "iot_log.h"
#include "iot_profile.h"

// format the report data to json string mode
static cJSON *FormateProflleValue(IoTProfileKVT *kv)
{
    cJSON  *ret = NULL;
    switch (kv->type) {
        case EN_IOT_DATATYPE_INT:
            ret = cJSON_CreateNumber(kv->iValue);
            break;
        case EN_IOT_DATATYPE_LONG:
            ret = cJSON_CreateNumber((double)(*(long *)kv->value));
            break;
        case EN_IOT_DATATYPE_FLOAT:
            ret = cJSON_CreateNumber((double)(*(float *)kv->fvalue));
            break;
        case EN_IOT_DATATYPE_DOUBLE:
            ret = cJSON_CreateNumber((*(double *)kv->dvalue));
            break;
        case EN_IOT_DATATYPE_STRING:
            ret = cJSON_CreateString((const char *)kv->value);
            break;
        default:
            break;
    }
    return ret;
}

static cJSON *MakeKvs(IoTProfileKVT *kvlst)
{
    cJSON *root;
    cJSON *kv;
    IoTProfileKVT *kvInfo;

    // build a root node
    root = cJSON_CreateObject();
    if (root == NULL) {
        return root;
    }

    // add all the property to the properties
    kvInfo = kvlst;
    while (kvInfo != NULL) {
        kv = FormateProflleValue(kvInfo);
        if (kv == NULL) {
            if (root != NULL) {
                cJSON_Delete(root);
                root = NULL;
            }
            return root;
        }

        cJSON_AddItemToObject(root, kvInfo->key, kv);
        kvInfo = kvInfo->nxt;
    }
    // OK, now we return it
    return root;
}

#define CN_PROFILE_SERVICE_KEY_SERVICEID "service_id"
#define CN_PROFILE_SERVICE_KEY_PROPERTIIES "properties"
#define CN_PROFILE_SERVICE_KEY_EVENTTIME "event_time"
#define CN_PROFILE_KEY_SERVICES "services"
static cJSON *MakeService(IoTProfileServiceT *serviceInfo)
{
    cJSON *root;
    cJSON *serviceID;
    cJSON *properties;
    cJSON *eventTime;

    // build a root node
    root = cJSON_CreateObject();
    if (root == NULL) {
        return root;
    }

    // add the serviceID node to the root node
    serviceID = cJSON_CreateString(serviceInfo->serviceID);
    if (serviceID == NULL) {
        if (root != NULL) {
            cJSON_Delete(root);
            root = NULL;
        }
        return root;
    }
    cJSON_AddItemToObjectCS(root, CN_PROFILE_SERVICE_KEY_SERVICEID, serviceID);

    // add the properties node to the root
    properties = MakeKvs(serviceInfo->serviceProperty);
    if (properties == NULL) {
        if (root != NULL) {
            cJSON_Delete(root);
            root = NULL;
        }
        return root;
    }
    cJSON_AddItemToObjectCS(root, CN_PROFILE_SERVICE_KEY_PROPERTIIES, properties);

    // add the event time (optional) to the root
    if (serviceInfo->eventTime != NULL) {
        eventTime = cJSON_CreateString(serviceInfo->eventTime);
        if (eventTime == NULL) {
            if (root != NULL) {
                cJSON_Delete(root);
                root = NULL;
            }
            return root;
        }
        cJSON_AddItemToObjectCS(root, CN_PROFILE_SERVICE_KEY_EVENTTIME, eventTime);
    }
    // OK, now we return it
    return root;
}

static cJSON *MakeServices(IoTProfileServiceT *serviceInfo)
{
    cJSON *services = NULL;
    cJSON *service;
    IoTProfileServiceT *serviceTmp;

    // create the services array node
    services = cJSON_CreateArray();
    if (services == NULL) {
        return services;
    }

    serviceTmp = serviceInfo;
    while (serviceTmp != NULL) {
        service = MakeService(serviceTmp);
        if (service == NULL) {
            return services;
        }
        cJSON_AddItemToArray(services, service);
        serviceTmp = serviceTmp->nxt;
    }

    // now we return the services
    return services;
}

// use this function to make a topic to publish
// if request_id  is needed depends on the fmt
static char *MakeTopic(const char *fmt, const char *deviceId, const char *requestID)
{
    int len;
    char *ret = NULL;

    len = strlen(fmt) + strlen(deviceId);
    if (requestID != NULL) {
        len += strlen(requestID);
    }
    ret = hi_malloc(0, len);
    if (ret != NULL) {
        if (requestID != NULL) {
            (void)snprintf_s(ret, len, len, fmt, deviceId, requestID);
        } else {
            (void)snprintf_s(ret, len, len, fmt, deviceId);
        }
    }
    return ret;
}

#define CN_PROFILE_CMDRESP_KEY_RETCODE    "result_code"
#define CN_PROFILE_CMDRESP_KEY_RESPNAME    "response_name"
#define CN_PROFILE_CMDRESP_KEY_PARAS    "paras"
static char *MakeProfileCmdResp(IoTCmdRespT *payload)
{
    char *ret = NULL;
    cJSON *root;
    cJSON *retCode;
    cJSON *respName;
    cJSON *paras;

    // create the root node
    root = cJSON_CreateObject();
    if (root == NULL) {
        return ret;
    }

    // create retcode and retdesc and add it to the root
    retCode = cJSON_CreateNumber(payload->retCode);
    if (retCode == NULL) {
        if (root != NULL) {
            cJSON_Delete(root);
        }
        return ret;
    }
    cJSON_AddItemToObjectCS(root, CN_PROFILE_CMDRESP_KEY_RETCODE, retCode);

    if (payload->respName != NULL) {
        respName = cJSON_CreateString(payload->respName);
        if (respName == NULL) {
            if (root != NULL) {
                cJSON_Delete(root);
            }
            return ret;
        }
        cJSON_AddItemToObjectCS(root, CN_PROFILE_CMDRESP_KEY_RESPNAME, respName);
    }

    if (payload->paras != NULL) {
        paras = MakeKvs(payload->paras);
        if (paras == NULL) {
            if (root != NULL) {
                cJSON_Delete(root);
            }
            return ret;
        }
        cJSON_AddItemToObjectCS(root, CN_PROFILE_CMDRESP_KEY_PARAS, paras);
    }

    // OK, now we make it to a buffer
    ret = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return ret;
}
#define CN_PROFILE_TOPICFMT_CMDRESP    "$oc/devices/%s/sys/commands/response/request_id=%s"
int IoTProfileCmdResp(const char *deviceID, IoTCmdRespT *payload)
{
    int ret = -1;
    const char *topic;
    const char *msg;

    if ((deviceID == NULL) || (payload == NULL) || (payload->requestID == NULL)) {
        return ret;
    }

    topic = MakeTopic(CN_PROFILE_TOPICFMT_CMDRESP, deviceID, payload->requestID);
    msg = MakeProfileCmdResp(payload);
    if ((topic != NULL) && (msg != NULL)) {
        ret = IotSendMsg(0, topic, msg);
    }

    hi_free(0, topic);
    cJSON_free(msg);
    return ret;
}

static char *MakeProfilePropertyReport(IoTProfileServiceT *payload)
{
    char *ret = NULL;
    cJSON *root;
    cJSON *services;

    // create the root node
    root = cJSON_CreateObject();
    if (root == NULL) {
        return ret;
    }

    // create the services array node to the root
    services = MakeServices(payload);
    if (services == NULL) {
        if (root != NULL) {
            cJSON_Delete(root);
        }
        return ret;
    }
    cJSON_AddItemToObjectCS(root, CN_PROFILE_KEY_SERVICES, services);

    // OK, now we make it to a buffer
    ret = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return ret;
}
#define CN_PROFILE_TOPICFMT_PROPERTYREPORT    "$oc/devices/%s/sys/properties/report"
int IoTProfilePropertyReport(char *deviceID, IoTProfileServiceT *payload)
{
    int ret = -1;
    char *topic;
    char *msg;

    if ((deviceID == NULL) || (payload == NULL) || (payload->serviceID == NULL) || (payload->serviceProperty == NULL)) {
        return ret;
    }
    topic = MakeTopic(CN_PROFILE_TOPICFMT_PROPERTYREPORT, deviceID, NULL);
    msg = MakeProfilePropertyReport(payload);
    if ((topic != NULL) && (msg != NULL)) {
        ret = IotSendMsg(0, topic, msg);
    }

    hi_free(0, topic);
    cJSON_free(msg);
    return ret;
}