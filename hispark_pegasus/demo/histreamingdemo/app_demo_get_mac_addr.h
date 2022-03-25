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

#ifndef APP_DEMO_GET_MAC_ADDR_H
#define APP_DEMO_GET_MAC_ADDR_H

#include <hi_types_base.h>

#define MAC_ADDR "%02x:%02x:%02x:%02x:%02x:%02x"
#define HISPARK_SSID_MIN_MAC_LEN   (6)
hi_u8 *GetSsid(void);
hi_u32 hi3816_get_mac_addr(hi_void);
#endif