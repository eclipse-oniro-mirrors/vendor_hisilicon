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
#include "securec.h"
#include "hi_flash.h"
#include "ohos_errno.h"
#include "ohos_types.h"
#include "hal_token.h"

#define BITS_PER_BYTE 8
// sector size is 4096 Bytes
#define SECTOR_ALIGN_BYTES 4096
#define TOKEN_SIZE 151

// using 256 Bytes to erase each token aera
#define MAX_TOKEN_AREA_SIZE 256

// 4 Bytes for token flag
// if token's both area are available, when read token, always return area which flag is bigger;
// and recover area which flag is small while write token.
#define TOKEN_FLAG_SIZE 4
#define TOKEN_WITH_FLAG_SIZE (TOKEN_SIZE + TOKEN_FLAG_SIZE)

#define TOKEN_ADDR 0x001F0000        // 实际的toekn地址

#define TOKEN_A_ADDR TOKEN_ADDR

#define TOKEN_B_ADDR (TOKEN_A_ADDR + SECTOR_ALIGN_BYTES)

#define TOKEN_DEBUG 1

#define HAL_TOKEN_OK 0
#define HAL_TOKEN_ERR (-1)
#define HAL_TOKEN_UNPRESET (-2)

#define FLASH_OPERAT_LEN 4
#define MALLOC_PARA 350
// 4 Bytes for token magic number, if data not in {1,2,3,4} order means the token area is not initialled.
// if token area is initialled, the token magic number's next Byte data is token actual value.
static const char g_tokenMagicNum[] = {1, 2, 3, 4};
#define TOKEN_MAGIC_NUM_SIZE (sizeof(g_tokenMagicNum) / sizeof(g_tokenMagicNum[0]))

#if TOKEN_DEBUG
#define TOKEN_LOG(...) printf(__VA_ARGS__)
#else
#define TOKEN_LOG(...)
#endif


static int32_t flashRead(uint32_t addr, uint32_t size, uint8_t *buffer)
{
    uint32_t len = 0;
    if ((size % FLASH_OPERAT_LEN) != 0) {
        len = size / FLASH_OPERAT_LEN * FLASH_OPERAT_LEN + FLASH_OPERAT_LEN;
    } else {
        len = size;
    }
    return hi_flash_read(addr, len, buffer);
}

static int32_t flashWrite(uint32_t addr, uint32_t size, uint8_t *buffer)
{
    uint32_t len = 0;
    if ((size % FLASH_OPERAT_LEN) != 0) {
        len = size / FLASH_OPERAT_LEN * FLASH_OPERAT_LEN + FLASH_OPERAT_LEN;
    } else {
        len = size;
    }
    return hi_flash_write(addr, len, buffer, 0);
}

static int32_t flashErase(uint32_t start_addr)
{
    return hi_flash_erase(start_addr, SECTOR_ALIGN_BYTES);
}

static int32_t FlashWriteTokenRawData(uint32_t start, const char* tokenRawData, uint32_t len)
{
    if (start % SECTOR_ALIGN_BYTES != 0) {
        printf("[FlashWriteTokenRawData]:Unsupport address not align yet, may cause data overlap error.\n");
        return HAL_TOKEN_ERR;
    }
    if (tokenRawData == NULL || len <= 0) {
        printf("[FlashWriteTokenRawData]:Invalid parameter.\n");
        return HAL_TOKEN_ERR;
    }
    if (flashErase(start) != 0) {
        return HAL_TOKEN_ERR;
    }
    return flashWrite(start, len, tokenRawData);
}

static int32_t FlashReadTokenRawData(uint32_t start, char* tokenRawData, uint32_t len)
{
    if (start % SECTOR_ALIGN_BYTES != 0) {
        printf("[FlashReadTokenRawData]:Unsupport start address not align yet, may cause data overlap error.\n");
        return HAL_TOKEN_ERR;
    }
    if (tokenRawData == NULL || len <= 0) {
        printf("[FlashReadTokenRawData]:Invalid parameter.\n");
        return HAL_TOKEN_ERR;
    }
    return flashRead(start, len, tokenRawData);
}

static int32_t ReadTokenWithFlag(uint32_t start, char* result, uint32_t len)
{
    const uint32_t buffLen = TOKEN_MAGIC_NUM_SIZE + TOKEN_WITH_FLAG_SIZE + 1;
    if (len < TOKEN_WITH_FLAG_SIZE) {
        return HAL_TOKEN_ERR;
    }
    char *buf = hi_malloc(MALLOC_PARA, buffLen);
    if (buf == NULL) {
        return HAL_TOKEN_ERR;
    }

    (void)memset_s(buf, buffLen, 0, buffLen);
    if (FlashReadTokenRawData(start, buf, buffLen) != 0) {
        printf("[ReadTokenWithFlag]:Read flash token area failed.\n");
        hi_free(MALLOC_PARA, buf);
        return HAL_TOKEN_ERR;
    }

    int32_t tokenValid = 1;
    // check is initialed or not
    for (uint32_t i = 0; i < TOKEN_MAGIC_NUM_SIZE; i++) {
        if (buf[i] != g_tokenMagicNum[i]) {
            tokenValid = 0;
            break;
        }
    }
    // token area is invalid
    if (tokenValid == 0) {
        printf("[ReadTokenWithFlag]:The token area is invalid.\n");
        hi_free(MALLOC_PARA, buf);
        return HAL_TOKEN_ERR;
    }
    if (memcpy_s(result, TOKEN_WITH_FLAG_SIZE, buf + TOKEN_MAGIC_NUM_SIZE, TOKEN_WITH_FLAG_SIZE) != 0) {
        return HAL_TOKEN_ERR;
    }
    hi_free(MALLOC_PARA, buf);
    printf("[ReadTokenWithFlag]:Read token success!\n");
    return HAL_TOKEN_OK;
}

static int32_t WriteTokenWithFlag(uint32_t start, const char* tokenWithFlag, uint32_t len)
{
    const uint32_t buffLen = TOKEN_MAGIC_NUM_SIZE + TOKEN_WITH_FLAG_SIZE + 1;
    char buf[buffLen];
    (void)memset_s(buf, buffLen, 0, buffLen);
    for (uint32_t i = 0; i < TOKEN_MAGIC_NUM_SIZE; i++) {
        buf[i] = g_tokenMagicNum[i];
    }
    if (memcpy_s(buf + TOKEN_MAGIC_NUM_SIZE, TOKEN_WITH_FLAG_SIZE, tokenWithFlag, TOKEN_WITH_FLAG_SIZE) != 0) {
        return HAL_TOKEN_ERR;
    }
    if (FlashWriteTokenRawData(start, buf, buffLen) != 0) {
        printf("[WriteTokenWithFlag]: Write flash token area failed.\n");
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

/* *
 * @brief Read token value from the flash token A or B area, and this function is only for token read and write.
 *
 * @param token The data buffer malloced by caller.
 * @param len The data buffer length.
 *
 * @returns -1 if it fails, the actual data is unknown.
 *          0 if it succeeds and means read token from area A or area B's data.
 *          -2 if it succeeds and means current is no token exist on the device.
 */
static int32_t OEMReadToken(char* token, uint32_t len)
{
    if (token == NULL || len == 0) {
        return HAL_TOKEN_ERR;
    }
    char tokenWithFlagA[TOKEN_WITH_FLAG_SIZE] = {0};
    char tokenWithFlagB[TOKEN_WITH_FLAG_SIZE] = {0};
    int32_t retA = ReadTokenWithFlag(TOKEN_A_ADDR, tokenWithFlagA, TOKEN_WITH_FLAG_SIZE);
    int32_t retB = ReadTokenWithFlag(TOKEN_B_ADDR, tokenWithFlagB, TOKEN_WITH_FLAG_SIZE);
    if ((retA != 0) && (retB != 0)) {
        printf("[OEMReadToken]:No token.\n");
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
    if (WriteTokenWithFlag(TOKEN_A_ADDR, tokenWithFlagA, TOKEN_WITH_FLAG_SIZE) != 0) {
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
    if (WriteTokenWithFlag(TOKEN_B_ADDR, tokenWithFlagB, TOKEN_WITH_FLAG_SIZE) != 0) {
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
    if (WriteTokenWithFlag(TOKEN_A_ADDR, tokenWithFlagA, TOKEN_WITH_FLAG_SIZE) != 0) {
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
        if (WriteTokenWithFlag(TOKEN_B_ADDR, tokenWithFlagB, TOKEN_WITH_FLAG_SIZE) != 0) {
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
        if (WriteTokenWithFlag(TOKEN_A_ADDR, tokenWithFlagA, TOKEN_WITH_FLAG_SIZE) != 0) {
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
 * @returns -1 if it fails, write token failed.
 *          0 if it succeeds and means write token to area A or area B's data.
 */
static int32_t OEMWriteToken(const char* token, uint32_t len)
{
    if ((token == NULL) || (len == 0)) {
        return HAL_TOKEN_ERR;
    }
    char tokenWithFlagA[TOKEN_WITH_FLAG_SIZE] = {0};
    char tokenWithFlagB[TOKEN_WITH_FLAG_SIZE] = {0};
    int32_t retA = ReadTokenWithFlag(TOKEN_A_ADDR, tokenWithFlagA, TOKEN_WITH_FLAG_SIZE);
    int32_t retB = ReadTokenWithFlag(TOKEN_B_ADDR, tokenWithFlagB, TOKEN_WITH_FLAG_SIZE);
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
    const char productIdBuf[] = "OH00004O";
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

