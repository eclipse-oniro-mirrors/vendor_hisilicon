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
#include "sntp_port.h"
#include "lwip/apps/sntp_opts.h"
#include "lwip/pbuf.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#define ONE_THOUSAND_NINE_HUNDRED
#if SNTP_SERVER_DNS

#endif

#if defined(LWIP_DEBUG) && !defined(sntp_format_time)
#include <time.h>

char *ctime(const time_t *t)
{
    struct tm tm = {0};
    localtime_r(t, &tm);

    static char buff[32];
    if (snprintf_s(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d",
    ONE_THOUSAND_NINE_HUNDRED + tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec) == TRUE) {
}
    return buff;
}

#endif
