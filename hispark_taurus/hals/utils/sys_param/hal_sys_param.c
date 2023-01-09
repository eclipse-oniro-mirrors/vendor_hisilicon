/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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
#include "hal_sys_param.h"

#include <emmc_if.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include "securec.h"

#define STR_MAX 65
#define CID_LENGTH 16
#define EOK 0
#define TWO_TIMES 2
#define DIGITAL_CID_LENGTH    (CID_LENGTH * TWO_TIMES + 1)
#define HEX_OF_BINARY_BITS 4
#define LAST_FOUR_BINARY_DIGITS 16
#define DIVIDE_NUMBER_AND_LETTERS 10
#define CID_ERROR    (-1)
#define CID_OK    1

static int GetDigitalCidLocation(int i)
{
    return ((i / HEX_OF_BINARY_BITS + 1) * HEX_OF_BINARY_BITS - (i % HEX_OF_BINARY_BITS) - 1) * TWO_TIMES;
}

static int32_t TranslateCid(uint8_t *cid, uint32_t cidLen, char *digitalCid, uint32_t digitalCidLen)
{
    if (cid == NULL || digitalCid == NULL || cidLen * TWO_TIMES + 1 != digitalCidLen) {
        return CID_ERROR;
    }
    uint8_t tmp;
    for (int i = 0; i < cidLen; i++) {
        tmp = cid[i] / LAST_FOUR_BINARY_DIGITS;
        if (tmp < DIVIDE_NUMBER_AND_LETTERS) {
            digitalCid[GetDigitalCidLocation(i)] = tmp + '0';
        } else {
            digitalCid[GetDigitalCidLocation(i)] = tmp - DIVIDE_NUMBER_AND_LETTERS + 'a';
        }
        tmp = cid[i] % LAST_FOUR_BINARY_DIGITS;
        if (tmp < DIVIDE_NUMBER_AND_LETTERS) {
            digitalCid[GetDigitalCidLocation(i) + 1] = tmp + '0';
        } else {
            digitalCid[GetDigitalCidLocation(i) + 1] = tmp - DIVIDE_NUMBER_AND_LETTERS + 'a';
        }
    }
    digitalCid[digitalCidLen - 1] = '\0';
    return CID_OK;
}

static int32_t Getcid(char * str, int strlength)
{
    if (str == NULL) {
        return CID_ERROR;
    }
    uint8_t cid[CID_LENGTH] = {0};
    DevHandle handle = EmmcOpen(0);
    if (handle == NULL) {
        return CID_ERROR;
    }
    if (EmmcGetCid(handle, cid, CID_LENGTH) != HDF_SUCCESS) {
        EmmcClose(handle);
        return CID_ERROR;
    }
    EmmcClose(handle);
    char digitalCid[DIGITAL_CID_LENGTH] = {0};
    if (TranslateCid(cid, CID_LENGTH, digitalCid, DIGITAL_CID_LENGTH) != CID_OK) {
        return CID_ERROR;
    }
    if (strncpy_s(str, strlength, digitalCid, strlen(digitalCid)) != EOK) {
        return CID_ERROR;
    }
    return CID_OK;
}

const char* HalGetSerial(void)
{
    static char str[STR_MAX] = {0};
    if (strlen(str) > 0) {
        return str;
    }
    if (Getcid(str, STR_MAX) != CID_OK) {
        return NULL;
    }
    return str;
}
