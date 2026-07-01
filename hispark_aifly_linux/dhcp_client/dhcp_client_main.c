/*
 * Copyright (c) 2025 HiSilicon (Shanghai) Technologies Co., Ltd.
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
#include <dlfcn.h>
#include <string.h>
#include <securec.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "samgr_lite.h"

#ifdef DEBUG_SERVICES_SAFWK_LITE
#include <sys/time.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

static volatile sig_atomic_t g_running = 1;

static void SigHandler(int sig)
{
    (void)sig;
    g_running = 0;
}

void __attribute__((weak)) OHOS_SystemInit(void)
{
    SAMGR_Bootstrap();
}

static void SystemExit(void)
{
    printf("[DHCP][I] DHCP client exiting gracefully.\n");
}

int main(int argc, char * const argv[])
{
    (void)argc;
    (void)argv;

#ifdef DEBUG_SERVICES_SAFWK_LITE
    printf("[Foundation][D] Start server system, begin. \n");

    struct timeval tvBefore;
    (void)gettimeofday(&tvBefore, NULL);
#endif

    struct sigaction sa;
    (void)memset_s(&sa, sizeof(sa), 0, sizeof(sa));
    sa.sa_handler = SigHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    OHOS_SystemInit();

#ifdef DEBUG_SERVICES_SAFWK_LITE
    struct timeval tvAfter;
    (void)gettimeofday(&tvAfter, NULL);

    printf("[Foundation][D] Start server system, end. duration %d seconds and %d microseconds. \n",\
        tvAfter.tv_sec - tvBefore.tv_sec, tvAfter.tv_usec - tvBefore.tv_usec);
#endif

    while (g_running) {
        // pause only returns when a signal was caught and the signal-catching function returned.
        // pause only returns -1, no need to process the return value.
        (void)pause();
    }

    SystemExit();
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
