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
#include <securec.h>

#define PROPERTY_MAX_LENGTH 2048

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
static const int OHOS_FIRST_API_VERSION = 1;

static const char EMPTY_STR[] = {""};

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
    return OHOS_SERIAL;
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

static const char* GetProperty(const char* propertyInfo, const size_t propertySize, const char** propertyHolder)
{
    if (*propertyHolder != NULL) {
        return *propertyHolder;
    }
    if ((propertySize == 0) || (propertySize > PROPERTY_MAX_LENGTH)) {
        return EMPTY_STR;
    }
    char* prop = (char*)malloc(propertySize);
    if (prop == NULL) {
        return EMPTY_STR;
    }
    if (strcpy_s(prop, propertySize, propertyInfo) != 0) {
        free(prop);
        prop = NULL;
        return EMPTY_STR;
    }
    *propertyHolder = prop;
    return *propertyHolder;
}

const char* HalGetIncrementalVersion(void)
{
    static const char* incrementalVersion = NULL;
    return GetProperty(INCREMENTAL_VERSION, strlen(INCREMENTAL_VERSION) + 1, &incrementalVersion);
}

const char* HalGetBuildType(void)
{
    static const char* buildType = NULL;
    return GetProperty(BUILD_TYPE, strlen(BUILD_TYPE) + 1, &buildType);
}

const char* HalGetBuildUser(void)
{
    static const char* buildUser = NULL;
    return GetProperty(BUILD_USER, strlen(BUILD_USER) + 1, &buildUser);
}

const char* HalGetBuildHost(void)
{
    static const char* buildHost = NULL;
    return GetProperty(BUILD_HOST, strlen(BUILD_HOST) + 1, &buildHost);
}

const char* HalGetBuildTime(void)
{
    static const char* buildTime = NULL;
    return GetProperty(BUILD_TIME, strlen(BUILD_TIME) + 1, &buildTime);
}

int HalGetFirstApiVersion(void)
{
    return OHOS_FIRST_API_VERSION;
}
