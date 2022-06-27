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
#ifndef SNTP_PORT_H
#define SNTP_PORT_H

#include "lwip/err.h"
#include "lwip/ip_addr.h"

typedef void (*__dns_found_callback)(const char *name, const ip_addr_t *ipaddr, int *arg);

err_t _dns_gethostbyname(const char *hostname, ip_addr_t *addr, __dns_found_callback found, int *arg);

#endif // SNTP_PORT_H
