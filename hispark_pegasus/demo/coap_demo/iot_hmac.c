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

/**
 * This file make use the hmac to make mqtt pwd.The method is use the date string to hash the device passwd .
 * Take care that this implement depends on the hmac of the mbedtls
*/
#include "iot_hmac.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "md.h"
#include "md_internal.h"

#define CN_HMAC256_LEN   32
#define HMAC256_LEN  65

// make a byte to 2 ascii hex
static int Byte2hexstr(unsigned char *bufin, int len, char *bufout)
{
    if ((bufin == NULL) || (len <= 0) || (bufout == NULL)) {
        return -1;
    }
    for (int i = 0; i < len; i++) {
        unsigned char tmpH = (bufin[i] >> 4) & 0X0F; /* 高字节前4位保存到tmpH */
        unsigned char tmpL = bufin[i] & 0x0F;
        bufout[2 * i] = (tmpH > 9) ? (tmpH - 10 + 'a') : (tmpH + '0'); /* 如果高字节大于9与高字节减10，将高字节转成字符形式,同时字符占2字节 */
        bufout[2 * i + 1] = (tmpL > 9) ? (tmpL - 10 + 'a') : (tmpL + '0'); /* 如果低字节大于9与高字节减10，将低字节转成字符形式，同时字符占2字节 */
    }
    bufout[2 * len] = '\0'; /* 字符占2字节 */
    return 0;
}

int HmacGeneratePwd(unsigned char *content, int contentLen, unsigned char *key, int keyLen,
                    unsigned char *buf)
{
    int ret = -1;
    mbedtls_md_context_t mbedtls_md_ctx;
    const mbedtls_md_info_t *mdInfo;
    unsigned char hash[CN_HMAC256_LEN];
    if (key == NULL || content == NULL || buf == NULL || keyLen == 0 || contentLen == 0 ||
        ((CN_HMAC256_LEN * 2 + 1) > HMAC256_LEN)) { /* 2倍的CN_HMAC256_LEN+1判断buflen是否合理 */
        return ret;
    }

    mdInfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (mdInfo == NULL || (size_t)mdInfo->size > CN_HMAC256_LEN) {
        return ret;
    }

    mbedtls_md_init(&mbedtls_md_ctx);
    ret = mbedtls_md_setup(&mbedtls_md_ctx, mdInfo, 1);
    if (ret != 0) {
        mbedtls_md_free(&mbedtls_md_ctx);
        return ret;
    }

    (void)mbedtls_md_hmac_starts(&mbedtls_md_ctx, key, keyLen);
    (void)mbedtls_md_hmac_update(&mbedtls_md_ctx, content, contentLen);
    (void)mbedtls_md_hmac_finish(&mbedtls_md_ctx, hash);

    // transfer the hash code to the string mode
    Byte2hexstr(hash, CN_HMAC256_LEN, (char *)buf);
    return ret;
}