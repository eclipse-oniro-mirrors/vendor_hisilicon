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

#ifndef IOT_PROFILE_H
#define IOT_PROFILE_H

#define OC_BEEP_STATUS_ON   ((hi_u8) 0x01)
#define OC_BEEP_STATUS_OFF  ((hi_u8) 0x00)

// //<enum all the data type for the oc profile
typedef enum {
    EN_IOT_DATATYPE_INT = 0,
    EN_IOT_DATATYPE_LONG,
    EN_IOT_DATATYPE_FLOAT,
    EN_IOT_DATATYPE_DOUBLE,
    EN_IOT_DATATYPE_STRING, // /<must be ended with '\0'
    EN_IOT_DATATYPE_LAST,
}IoTDataTypeT;

typedef enum {
    OC_LED_ON = 1,
    OC_LED_OFF
}OcLedValue;

typedef struct {
    void    *nxt; // /<ponit to the next key
    const char    *key;
    const char    *value;
    double *dvalue;
    float *fvalue;
    int    iValue;
    IoTDataTypeT    type;
}IoTProfileKVT;

typedef struct {
    void    *nxt;
    char    *serviceID; // /<the service id in the profile, which could not be NULL
    char    *eventTime; // /<eventtime, which could be NULL means use the platform time
    IoTProfileKVT    *serviceProperty; // /<the property in the profile, which could not be NULL
}IoTProfileServiceT;

typedef struct {
    int    retCode; // /<response code, 0 success while others failed
    const char    *respName; // /<response name
    const char    *requestID; // /<specified by the message command
    IoTProfileKVT    *paras; // /<the command paras
}IoTCmdRespT;
/*
 * Use this function to make the command response here
 * and you must supplied the device id, and the payload defines as IoTCmdResp_t
 */
int IoTProfileCmdResp(const char *deviceID, IoTCmdRespT *payload);

/*
 * use this function to report the property to the iot platform
 */
int IoTProfilePropertyReport(char *deviceID, IoTProfileServiceT *payload);


#endif