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

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include "hal_sys_param.h"

#define STR_MAX  65

static const char OHOS_DEVICE_TYPE[] = {"****"};
static const char OHOS_DISPLAY_VERSION[] = {"OpenHarmony 1.0.1"};
static const char OHOS_MANUFACTURE[] = {"****"};
static const char OHOS_BRAND[] = {"****"};
static const char OHOS_MARKET_NAME[] = {"****"};
static const char OHOS_PRODUCT_SERIES[] = {"****"};
static const char OHOS_PRODUCT_MODEL[] = {"****"};
static const char OHOS_SOFTWARE_MODEL[] = {"****"};
static const char OHOS_HARDWARE_MODEL[] = {"****"};
static const char OHOS_HARDWARE_PROFILE[] = {"aout:true,display:true"};
static const char OHOS_BOOTLOADER_VERSION[] = {"bootloader"};
static const char OHOS_ABI_LIST[] = {"****"};
static const char OHOS_SERIAL[] = {"1234567890"};  // provided by OEM.
static const char SN_FILE[] = {"/sys/block/mmcblk0/device/cid"};
static const int OHOS_FIRST_API_VERSION = 1;

const char* HalGetDeviceType(void)
{
    return OHOS_DEVICE_TYPE;
}

const char* HalGetManufacture(void)
{
    return OHOS_MANUFACTURE;
}

const char* HalGetBrand(void)
{
    return OHOS_BRAND;
}

const char* HalGetMarketName(void)
{
    return OHOS_MARKET_NAME;
}

const char* HalGetProductSeries(void)
{
    return OHOS_PRODUCT_SERIES;
}

const char* HalGetProductModel(void)
{
    return OHOS_PRODUCT_MODEL;
}

const char* HalGetSoftwareModel(void)
{
    return OHOS_SOFTWARE_MODEL;
}

const char* HalGetHardwareModel(void)
{
    return OHOS_HARDWARE_MODEL;
}

const char* HalGetHardwareProfile(void)
{
    return OHOS_HARDWARE_PROFILE;
}

const char* HalGetSerial(void)
{
    char strOrigin[STR_MAX] = {0};
    static char str[STR_MAX] = {0};
    if (strlen(str) > 0) {
        return str;
    }
    int fd = open(SN_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        return OHOS_SERIAL;
    }
    int ret = read(fd, strOrigin, STR_MAX - 1);
    if (ret <= 0) {
        close(fd);
        return OHOS_SERIAL;
    }
    close(fd);
    int j = 0;
    for (int i = 0; strOrigin[i] != '\0' && i < STR_MAX; i++) {
        if ((strOrigin[i] <= '9' && strOrigin[i] >= '0') || (strOrigin[i] <= 'f' && strOrigin[i] >= 'a')) {
            str[j] = strOrigin[i];
            j++;
        }
    }
    str[j] = '\0';
    return str;
}

const char* HalGetBootloaderVersion(void)
{
    return OHOS_BOOTLOADER_VERSION;
}

const char* HalGetAbiList(void)
{
    return OHOS_ABI_LIST;
}

const char* HalGetDisplayVersion(void)
{
    return OHOS_DISPLAY_VERSION;
}

const char* HalGetIncrementalVersion(void)
{
    return INCREMENTAL_VERSION;
}

const char* HalGetBuildType(void)
{
    return BUILD_TYPE;
}

const char* HalGetBuildUser(void)
{
    return BUILD_USER;
}

const char* HalGetBuildHost(void)
{
    return BUILD_HOST;
}

const char* HalGetBuildTime(void)
{
    return BUILD_TIME;
}

int HalGetFirstApiVersion(void)
{
    return OHOS_FIRST_API_VERSION;
}
