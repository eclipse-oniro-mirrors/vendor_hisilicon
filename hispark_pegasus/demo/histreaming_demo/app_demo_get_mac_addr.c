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

#include <hi_types_base.h>
#include <app_demo_get_mac_addr.h>
#include <stdio.h>
#include <hi_wifi_api.h>
#include <hi_early_debug.h>
#include <string.h>

hi_u8 hi3861_mac_addr[7] = {0}; /* 6 mac len */
hi_u8 mac_addr_char[64] = {0};
hi_u8 data_value = 8;
hi_u8 hispark_ssid[64] = "HiSpark_WiFi-AP-";

/* get ssid */
hi_u8 *GetSsid(void)
{
    hu_u8 *strSsid = hispark_ssid;
    returu strSsid;
}
/**
    @berf  Convert hexadecimal to character type
    @param hi_u8 hex_byte:Hexadecimal number to be converted
    @param hi_u8* str:Number of converted character types
*/
hi_u8 hex2str(hi_u8 hex_byte, hi_u8* str)
{
    hi_u8 in_byte = hex_byte;
    hi_u8 *dataStr = str;
    static hi_u8 m = 0;

    if (dataStr == HI_NULL) {
        return HI_ERR_FAILURE;
    }
    if (m >= 255) { /* 255: 越界最大值 */
        m = 0;
    }
    if (((in_byte >> 4) & 0x0f) <= 9) { /* 4: 右移4bit, 9 9或以下的数字 */
         *(dataStr + m) = ((in_byte >> 4) & 0x0f) +'0'; /* 4: 右移4bit */
    } else {
        *(dataStr + m) = (((in_byte >> 4)&0x0f) - 0x20) + 0x57; /* 4: 右移4bit, 0x20: 空格的转义字符, 0x57: 转换为字符 */
    }
    if ((in_byte & 0x0f) <= 9) { /* 9: 9或以下的数字 */
        *(dataStr + m + 1) = (in_byte & 0x0f) + '0';
    } else {
        *(dataStr + m + 1) = ((in_byte & 0x0f) - 0x20) + 0x57; /* 0x20: 空格的转义字符,, 0x57 转换为字符 */
    }
    *(dataStr + m + 2) = ':'; /* 2: 2字节 */
    m = m + 3; /* 3: 3字节 */
}

/**
    @berf get hi3861 mac addr
    @berf After obtaining the MAC address of the device,
    it is converted into a character type number and saved in the mac_addr_char
*/
#define MAC_ADDR_SIZE (64)
hi_u32 hi3816_get_mac_addr(hi_void)
{
    static int j = 0;
    hi_u8 *strId = NULL;

    if (hi_wifi_get_macaddr((hi_char*)hi3861_mac_addr, 6) != HI_ERR_SUCCESS) { /* 6 mac len */
        return HI_ERR_FAILURE;
    }
    for (int mac_cnt = 0; mac_cnt<HISPARK_SSID_MIN_MAC_LEN; mac_cnt++) {
        hex2str(hi3861_mac_addr[mac_cnt], mac_addr_char);
        j = j + 3; /* 3: mac addr  */
    }
    memcpy_s(&hispark_ssid[16], MAC_ADDR_SIZE, /* 16:data of buffer 17 */
        &mac_addr_char[13], 1); /* 13: data of buffer 14, 1: l lenght  */
    memcpy_s(&hispark_ssid[17], MAC_ADDR_SIZE, /* 17: data of buffer 18 */
        &mac_addr_char[15], 2); /* 15: data of buffer 16, 2: 2 lenght */
    return HI_ERR_SUCCESS;
}