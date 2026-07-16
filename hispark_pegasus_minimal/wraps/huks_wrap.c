/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <stdint.h>
#include "hi_cipher.h"

#define HKS_ERROR_NULL_POINTER          ((int32_t)-1000)
#define HKS_ERROR_INVALID_ARGUMENT      ((int32_t)-135)
#define HKS_MAX_RANDOM_LEN              1024

struct hks_blob {
    uint8_t type;
    uint8_t *data;
    uint32_t size;
};

int32_t hks_generate_random(struct hks_blob *random)
{
    if ((random == NULL) || (random->data == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }
    if ((random->size == 0) || (random->size > HKS_MAX_RANDOM_LEN)) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    if (hi_cipher_trng_get_random_bytes(random->data, random->size) != 0) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    return 0;
}
