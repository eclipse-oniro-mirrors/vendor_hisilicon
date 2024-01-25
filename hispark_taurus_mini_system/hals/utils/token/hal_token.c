/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <securec.h>
#include <unistd.h>
#include <pthread.h>
#include "ohos_errno.h"
#include "ohos_types.h"
#include "hal_token.h"

#define BITS_PER_BYTE 8
// sector size is 4096 Bytes
#define SECTOR_ALIGN_BYTES 4096
// token's max length is 151 Bytes, we using 256 Bytes to erase each token aera
#define MAX_TOKEN_AREA_SIZE 256
#define TOKEN_SIZE 151
// 4 Bytes for token flag
// if token's both area are available, when read token, always return area which flag is bigger;
// and recover area which flag is small while write token.
#define TOKEN_FLAG_SIZE 4
#define TOKEN_WITH_FLAG_SIZE (TOKEN_SIZE + TOKEN_FLAG_SIZE)
// 4 Bytes for token magic number, if data not in {1,2,3,4} order means the token area is not initialled.
// if token area is initialled, the token magic number's next Byte data is token actual value.
static const char g_tokenMagicNum[] = {1, 2, 3, 4};
#define TOKEN_MAGIC_NUM_SIZE (sizeof(g_tokenMagicNum) / sizeof(g_tokenMagicNum[0]))

#define TOKEN_FILE_PATH "/storage/data/device_attest"
#define TOKEN_A_FILE_NAME "tokenA"
#define TOKEN_B_FILE_NAME "tokenB"
#define PATH_MAX  255

#define HAL_TOKEN_OK (0)
#define HAL_TOKEN_ERR (-1)
#define HAL_TOKEN_UNPRESET (-2)

pthread_mutex_t tokenMutex = PTHREAD_MUTEX_INITIALIZER; // 创建互斥锁
static int tokenLock = 0;

static char* GenTokenFilePath(const char* dirPath, const char* fileName)
{
    if (dirPath == NULL || fileName == NULL) {
        return NULL;
    }

    uint32_t filePathLen = strlen(dirPath) + 1 + strlen(fileName) + 1;
    if (filePathLen > PATH_MAX) {
        return NULL;
    }
    char* filePath = (char *)malloc(filePathLen);
    if (filePath == NULL) {
        return NULL;
    }

    (void)memset_s(filePath, filePathLen, 0, filePathLen);
    if (sprintf_s(filePath, filePathLen, "%s%s%s", dirPath, "/", fileName) < 0) {
        free(filePath);
        return NULL;
    }
    return filePath;
}

static int32_t GetTokenFileSize(const char* path, const char* fileName, uint32_t* result)
{
    if (path == NULL || fileName == NULL || result == NULL) {
        return HAL_TOKEN_ERR;
    }

    char* filePath = GenTokenFilePath(path, fileName);
    if (filePath == NULL) {
        return HAL_TOKEN_ERR;
    }

    char* formatPath = realpath(filePath, NULL);
    if (formatPath == NULL) {
        return HAL_TOKEN_ERR;
    }

    FILE* fp = fopen(formatPath, "r");
    if (fp == NULL) {
        free(formatPath);
        return HAL_TOKEN_ERR;
    }
    if (fseek(fp, 0, SEEK_END) < 0) {
        free(formatPath);
        (void)fclose(fp);
        return HAL_TOKEN_ERR;
    }
    *result = ftell(fp);
    free(formatPath);
    (void)fclose(fp);
    return HAL_TOKEN_OK;
}

static int32_t ReadTokenFile(const char* path, const char* fileName, char* buffer, uint32_t bufferLen)
{
    if (path == NULL || fileName == NULL || buffer == NULL || bufferLen == 0) {
        return HAL_TOKEN_ERR;
    }

    uint32_t fileSize = 0;
    if (GetTokenFileSize(path, fileName, &fileSize) != 0 || fileSize > bufferLen) {
        return HAL_TOKEN_ERR;
    }

    char* filePath = GenTokenFilePath(path, fileName);
    if (filePath == NULL) {
        return HAL_TOKEN_ERR;
    }

    char* formatPath = realpath(filePath, NULL);
    free(filePath);
    if (formatPath == NULL) {
        return HAL_TOKEN_ERR;
    }

    FILE* fp = fopen(formatPath, "rb");
    if (fp == NULL) {
        free(formatPath);
        return HAL_TOKEN_ERR;
    }
    if (fread(buffer, fileSize, 1, fp) != 1) {
        free(formatPath);
        (void)fclose(fp);
        return HAL_TOKEN_ERR;
    }
    free(formatPath);
    (void)fclose(fp);
    return HAL_TOKEN_OK;
}

static int32_t WriteTokenFile(const char* path, const char* fileName, const char* data, uint32_t dataLen)
{
    if (path == NULL || fileName == NULL || data == NULL || dataLen == 0) {
        return HAL_TOKEN_ERR;
    }

    char* formatPath = realpath(path, NULL);
    if (formatPath == NULL) {
        return HAL_TOKEN_ERR;
    }

    char* filePath = GenTokenFilePath(formatPath, fileName);
    free(formatPath);
    if (filePath == NULL) {
        return HAL_TOKEN_ERR;
    }

    FILE* fp = fopen(filePath, "wb+");
    if (fp == NULL) {
        free(filePath);
        return HAL_TOKEN_ERR;
    }
    if (fwrite(data, dataLen, 1, fp) != 1) {
        (void)fclose(fp);
        free(filePath);
        return HAL_TOKEN_ERR;
    }
    (void)fclose(fp);
    free(filePath);
    return HAL_TOKEN_OK;
}

static int32_t ReadTokenRawData(const char* path, const char* fileName, char* buffer, uint32_t bufferLen)
{
    tokenLock = pthread_mutex_lock(&tokenMutex);
    if (tokenLock == 0) {
        int ret = ReadTokenFile(path, fileName, buffer, bufferLen);
        if (ret < 0) {
            pthread_mutex_unlock(&tokenMutex);
            return HAL_TOKEN_ERR;
        }
    }
    pthread_mutex_unlock(&tokenMutex);
    return HAL_TOKEN_OK;
}

static int32_t WriteTokenRawData(const char* path, const char* fileName, const char* data, uint32_t dataLen)
{
    tokenLock = pthread_mutex_lock(&tokenMutex);
    if (tokenLock == 0) {
        int ret = WriteTokenFile(path, fileName, data, dataLen);
        if (ret < 0) {
            pthread_mutex_unlock(&tokenMutex);
            return HAL_TOKEN_ERR;
        }
    }
    pthread_mutex_unlock(&tokenMutex);
    return HAL_TOKEN_OK;
}

static int32_t ReadTokenWithFlag(const char* path, const char* fileName, char* TokenWithFlag, uint32_t len)
{
    const uint32_t buffLen = TOKEN_MAGIC_NUM_SIZE + TOKEN_WITH_FLAG_SIZE + 1;
    if (len < TOKEN_WITH_FLAG_SIZE) {
        return HAL_TOKEN_ERR;
    }
    char *buf = malloc(buffLen);
    if (buf == NULL) {
        return HAL_TOKEN_ERR;
    }

    (void)memset_s(buf, buffLen, 0, buffLen);

    if (ReadTokenRawData(path, fileName, buf, buffLen) != 0) {
        free(buf);
        return HAL_TOKEN_ERR;
    }
    int32_t tokenValid = 1;

    for (uint32_t i = 0; i < TOKEN_MAGIC_NUM_SIZE; i++) {
        if (buf[i] != g_tokenMagicNum[i]) {
            tokenValid = 0;
            break;
        }
    }
    if (tokenValid == 0) {
        free(buf);
        return HAL_TOKEN_ERR;
    }
    if (memcpy_s(TokenWithFlag, TOKEN_WITH_FLAG_SIZE, buf + TOKEN_MAGIC_NUM_SIZE, TOKEN_WITH_FLAG_SIZE) != 0) {
        return HAL_TOKEN_ERR;
    }
    free(buf);
    return HAL_TOKEN_OK;
}

static int32_t WriteTokenWithFlag(const char* path, const char* fileName, const char* TokenWithFlag, uint32_t len)
{
    const uint32_t buffLen = TOKEN_MAGIC_NUM_SIZE + TOKEN_WITH_FLAG_SIZE + 1;
    char buf[buffLen];
    (void)memset_s(buf, buffLen, 0, buffLen);

    for (uint32_t i = 0; i < TOKEN_MAGIC_NUM_SIZE; i++) {
        buf[i] = g_tokenMagicNum[i];
    }
    if (memcpy_s(buf + TOKEN_MAGIC_NUM_SIZE, TOKEN_WITH_FLAG_SIZE, TokenWithFlag, TOKEN_WITH_FLAG_SIZE) != 0) {
        return HAL_TOKEN_ERR;
    }
    if (WriteTokenRawData(path, fileName, buf, len) != 0) {
        return HAL_TOKEN_ERR;
    }
    return HAL_TOKEN_OK;
}

static uint32_t GetTokenFlag(const char tokenWithFlag[])
{
    uint32_t result = 0;
    for (uint32_t i = 0; i < TOKEN_FLAG_SIZE; i++) {
        result |= ((uint8_t)tokenWithFlag[TOKEN_SIZE + i]) << ((TOKEN_FLAG_SIZE - 1 - i) * BITS_PER_BYTE);
    }
    return result;
}
static void SetTokenFlag(uint8_t flag[], uint32_t value)
{
    for (uint32_t i = 0; i < TOKEN_FLAG_SIZE; i++) {
        flag[i] = (value >> (BITS_PER_BYTE * (TOKEN_FLAG_SIZE - 1 - i))) & 0xFF;
    }
}

static int32_t OEMReadToken(char* token, uint32_t len)
{
    if (token == NULL || len == 0) {
        return HAL_TOKEN_ERR;
    }
    char tokenWithFlagA[TOKEN_WITH_FLAG_SIZE] = {0};
    char tokenWithFlagB[TOKEN_WITH_FLAG_SIZE] = {0};
    int32_t retA = ReadTokenWithFlag(TOKEN_FILE_PATH, TOKEN_A_FILE_NAME, tokenWithFlagA, TOKEN_WITH_FLAG_SIZE);
    int32_t retB = ReadTokenWithFlag(TOKEN_FILE_PATH, TOKEN_B_FILE_NAME, tokenWithFlagB, TOKEN_WITH_FLAG_SIZE);
    if ((retA != 0) && (retB != 0)) {
        // -2 means current is no token exist on the device
        return HAL_TOKEN_UNPRESET;
    } else if ((retA == 0) && (retB != 0)) {
        // token area A has data, area B is NULL, return A;
        return memcpy_s(token, len, tokenWithFlagA, len);
    } else if ((retA != 0) && (retB == 0)) {
        // token area B has data, area A is NULL, return B;
        return memcpy_s(token, len, tokenWithFlagB, len);
    } else {
        // token area A and B both have data, return area which flag is larger than the other one.
        uint32_t flagA = GetTokenFlag(tokenWithFlagA);
        uint32_t flagB = GetTokenFlag(tokenWithFlagB);
        if (flagA > flagB) {
            return memcpy_s(token, len, tokenWithFlagA, len);
        } else {
            return memcpy_s(token, len, tokenWithFlagB, len);
        }
    }
    return HAL_TOKEN_OK;
}

static int32_t OEMWriteTokenANoToken(const char* token, uint32_t len, char* tokenWithFlagA)
{
    if (tokenWithFlagA == NULL) {
        printf("[OEMWriteTokenANoToken]Invalid parameter.\n");
        return HAL_TOKEN_ERR;
    }
    uint8_t flag[TOKEN_FLAG_SIZE] = {0};
    if ((memcpy_s(tokenWithFlagA, TOKEN_WITH_FLAG_SIZE, token, len) != 0) ||
        (memcpy_s(tokenWithFlagA + len, TOKEN_WITH_FLAG_SIZE - len, flag, TOKEN_FLAG_SIZE) != 0)) {
        printf("[OEMWriteTokenANoToken]:Flash write token memcpy failed.\n");
        return HAL_TOKEN_ERR;
    }
    if (WriteTokenWithFlag(TOKEN_FILE_PATH, TOKEN_A_FILE_NAME, tokenWithFlagA, TOKEN_WITH_FLAG_SIZE) != 0) {
        printf("[OEMWriteTokenANoToken]:Flash write token area A failed.\n");
        return HAL_TOKEN_ERR;
    }
    return HAL_TOKEN_OK;
}

static int32_t OEMWriteTokenB(const char* token, uint32_t len, char* tokenWithFlagA, char* tokenWithFlagB)
{
    if (tokenWithFlagA == NULL || tokenWithFlagB == NULL) {
        printf("[OEMWriteTokenB]Invalid parameter.\n");
        return HAL_TOKEN_ERR;
    }
    uint32_t flagA = GetTokenFlag(tokenWithFlagA);
    uint8_t flag[TOKEN_FLAG_SIZE] = {0};
    SetTokenFlag(flag, (uint32_t)(flagA + 1));
    (void)memset_s(tokenWithFlagB, TOKEN_WITH_FLAG_SIZE, 0, TOKEN_WITH_FLAG_SIZE);
    if ((memcpy_s(tokenWithFlagB, TOKEN_WITH_FLAG_SIZE, token, len) != 0) ||
        (memcpy_s(tokenWithFlagB + len, TOKEN_WITH_FLAG_SIZE, flag, TOKEN_FLAG_SIZE) != 0)) {
        printf("[OEMWriteTokenB]:Flash write token memcpy failed.\n");
        return HAL_TOKEN_ERR;
    }
    if (WriteTokenWithFlag(TOKEN_FILE_PATH, TOKEN_B_FILE_NAME, tokenWithFlagB, TOKEN_WITH_FLAG_SIZE) != 0) {
        printf("[OEMWriteTokenB]:Flash write token area B failed.\n");
        return HAL_TOKEN_ERR;
    }
    return HAL_TOKEN_OK;
}

static int32_t OEMWriteTokenA(const char* token, uint32_t len, char* tokenWithFlagA, char* tokenWithFlagB)
{
    if (tokenWithFlagA == NULL || tokenWithFlagB == NULL) {
        printf("[OEMWriteTokenA]Invalid parameter.\n");
        return HAL_TOKEN_ERR;
    }
    uint32_t flagB = GetTokenFlag(tokenWithFlagB);
    uint8_t flag[TOKEN_FLAG_SIZE] = {0};
    SetTokenFlag(flag, (uint32_t)(flagB + 1));
    (void)memset_s(tokenWithFlagA, TOKEN_WITH_FLAG_SIZE, 0, TOKEN_WITH_FLAG_SIZE);
    if ((memcpy_s(tokenWithFlagA, TOKEN_WITH_FLAG_SIZE, token, len) != 0) ||
        (memcpy_s(tokenWithFlagA + len, TOKEN_WITH_FLAG_SIZE, flag, TOKEN_FLAG_SIZE) != 0)) {
        printf("[OEMWriteTokenA]:Flash write token memcpy failed.\n");
        return HAL_TOKEN_ERR;
    }
    if (WriteTokenWithFlag(TOKEN_FILE_PATH, TOKEN_A_FILE_NAME, tokenWithFlagA, TOKEN_WITH_FLAG_SIZE) != 0) {
        printf("[OEMWriteTokenA]:Flash write token area A failed.\n");
        return HAL_TOKEN_ERR;
    }
    return HAL_TOKEN_OK;
}

static int32_t OEMWriteTokenSmaller(const char* token, uint32_t len, char* tokenWithFlagA, char* tokenWithFlagB)
{
    if (tokenWithFlagA == NULL || tokenWithFlagB == NULL) {
        printf("[OEMWriteTokenSmaller]Invalid parameter.\n");
        return HAL_TOKEN_ERR;
    }
    uint32_t flagA = GetTokenFlag(tokenWithFlagA);
    uint32_t flagB = GetTokenFlag(tokenWithFlagB);
    if (flagA > flagB) {
        uint8_t flag[TOKEN_FLAG_SIZE] = {0};
        SetTokenFlag(flag, (uint32_t)(flagA + 1));

        // area A's token is new, recover area B;
        (void)memset_s(tokenWithFlagB, TOKEN_WITH_FLAG_SIZE, 0, TOKEN_WITH_FLAG_SIZE);
        if ((memcpy_s(tokenWithFlagB, TOKEN_WITH_FLAG_SIZE, token, len) != 0) ||
            (memcpy_s(tokenWithFlagB + len, TOKEN_WITH_FLAG_SIZE, flag, TOKEN_FLAG_SIZE) != 0)) {
            printf("[OEMWriteTokenSmaller]:Flash write tokenB memcpy failed.\n");
            return HAL_TOKEN_ERR;
        }
        if (WriteTokenWithFlag(TOKEN_FILE_PATH, TOKEN_B_FILE_NAME, tokenWithFlagB, TOKEN_WITH_FLAG_SIZE) != 0) {
            printf("[OEMWriteTokenSmaller]:Flash write token area B failed.\n");
            return HAL_TOKEN_ERR;
        }
        return HAL_TOKEN_OK;
    } else {
        uint8_t flag[TOKEN_FLAG_SIZE] = {0};
        SetTokenFlag(flag, (uint32_t)(flagB + 1));

        // area B's token is new, recover area A;
        (void)memset_s(tokenWithFlagA, TOKEN_WITH_FLAG_SIZE, 0, TOKEN_WITH_FLAG_SIZE);
        if ((memcpy_s(tokenWithFlagA, TOKEN_WITH_FLAG_SIZE, token, len) != 0) ||
            (memcpy_s(tokenWithFlagA + len, TOKEN_WITH_FLAG_SIZE, flag, TOKEN_FLAG_SIZE) != 0)) {
            printf("[OEMWriteTokenSmaller]:Flash write tokenA memcpy failed.\n");
            return HAL_TOKEN_ERR;
        }
        if (WriteTokenWithFlag(TOKEN_FILE_PATH, TOKEN_A_FILE_NAME, tokenWithFlagA, TOKEN_WITH_FLAG_SIZE) != 0) {
            printf("[OEMWriteTokenSmaller]:Flash write token area A failed.\n");
            return HAL_TOKEN_ERR;
        }
        return HAL_TOKEN_OK;
    }
}
/* *
 * @brief Write token value to the token A or B area on the flash, and this function is only for token read and write.
 *
 * @param token The input token data.
 * @param len The token's length.
 *
 * @returns    -1 if it fails, write token failed.
 * 0 if it succeeds and means write token to area A or area B's data.
 */
static int32_t OEMWriteToken(const char* token, uint32_t len)
{
    if ((token == NULL) || (len == 0)) {
        return HAL_TOKEN_ERR;
    }
    char tokenWithFlagA[TOKEN_WITH_FLAG_SIZE] = {0};
    char tokenWithFlagB[TOKEN_WITH_FLAG_SIZE] = {0};
    int32_t retA = ReadTokenWithFlag(TOKEN_FILE_PATH, TOKEN_A_FILE_NAME, tokenWithFlagA, TOKEN_WITH_FLAG_SIZE);
    int32_t retB = ReadTokenWithFlag(TOKEN_FILE_PATH, TOKEN_B_FILE_NAME, tokenWithFlagB, TOKEN_WITH_FLAG_SIZE);
    if ((retA != 0) && (retB != 0)) {
        printf("[OEMWriteToken]:No token data on device.\n");
        return OEMWriteTokenANoToken(token, len, tokenWithFlagA);
    } else if ((retA == 0) && (retB != 0)) {
        // token area A has data, area B is NULL, write token to B area;
        return OEMWriteTokenB(token, len, tokenWithFlagA, tokenWithFlagB);
    } else if ((retA != 0) && (retB == 0)) {
        // write token to A area
        return OEMWriteTokenA(token, len, tokenWithFlagA, tokenWithFlagB);
    } else {
        // write token to the area which flag is smaller than the other one.
        return OEMWriteTokenSmaller(token, len, tokenWithFlagA, tokenWithFlagB);
    }
}

int HalReadToken(char *token, unsigned int len)
{
    if (token == NULL) {
        return EC_FAILURE;
    }
    return OEMReadToken(token, len);
}

int HalWriteToken(const char *token, unsigned int len)
{
    if (token == NULL) {
        return EC_FAILURE;
    }
    return OEMWriteToken(token, len);
}

int HalGetAcKey(char *acKey, unsigned int len)
{
    if ((acKey == NULL) || (len == 0)) {
        return EC_FAILURE;
    }
    const char manufacturekeyBuf[] = {
        0x13, 0x42, 0x3F, 0x3F, 0x53, 0x3F, 0x72, 0x30, 0x3F, 0x3F, 0x1C, 0x3F, 0x2F, 0x3F, 0x2E, 0x42,
        0x3F, 0x08, 0x3F, 0x57, 0x3F, 0x10, 0x3F, 0x3F, 0x29, 0x17, 0x52, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
        0x57, 0x16, 0x3F, 0x7D, 0x4A, 0x0F, 0x3F, 0x3F, 0x3F, 0x30, 0x0C, 0x3F, 0x3F, 0x4C, 0x3F, 0x47
    };
    uint32_t manufacturekeyBufLen = sizeof(manufacturekeyBuf);
    if (len < manufacturekeyBufLen) {
        return EC_FAILURE;
    }

    int ret = memcpy_s(acKey, len, manufacturekeyBuf, manufacturekeyBufLen);
    return ret;
}

int HalGetProdId(char *productId, unsigned int len)
{
    if ((productId == NULL) || (len == 0)) {
        return EC_FAILURE;
    }
    const char productIdBuf[] = "OH00000D";
    uint32_t productIdLen = strlen(productIdBuf);
    if (len < productIdLen) {
        return EC_FAILURE;
    }

    int ret = memcpy_s(productId, len, productIdBuf, productIdLen);
    return ret;
}

int HalGetProdKey(char *productKey, unsigned int len)
{
    if ((productKey == NULL) || (len == 0)) {
        return EC_FAILURE;
    }
    const char productKeyBuf[] = "test";
    uint32_t productKeyLen = sizeof(productKeyBuf);
    if (len < productKeyLen) {
        return EC_FAILURE;
    }

    int ret = memcpy_s(productKey, len, productKeyBuf, productKeyLen);
    return ret;
}