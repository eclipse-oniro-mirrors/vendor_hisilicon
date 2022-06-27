/*
 * Copyright (C) 2022 HiHope Open Source Organization .
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http:// www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 *
 * limitations under the License.
 */
#ifndef CONFIG_PARAMS_H
#define CONFIG_PARAMS_H

// kv_store key
#define PARAM_HOTSPOT_SSID_KEY "hostpot_ssid"
#define PARAM_HOTSPOT_PSK_KEY "hostpot_psk"
#define PARAM_LIGHTNESS "lightness"
#define PARAM_LIGHT_TIME "time"
#define PARAM_TEMP_MAX "temp_max"
#define PARAM_TEMP_MIN "temp_min"
#define PARAM_HUMI_MAX "humi_max"
#define PARAM_HUMI_MIN "humi_min"
#define PARAM_GAS_VALUE "gas_value"

typedef enum {
    WIFI_AP = 0,
    WIFI_STA = 1,
} WifiStatus;

#endif // CONFIG_PARAMS_H
