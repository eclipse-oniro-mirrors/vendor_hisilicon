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

#ifndef NET_DEMO_COMMON_H
#define NET_DEMO_COMMON_H

void NetDemoTest(unsigned short port, const char* host);

const char* GetNetDemoName(void);

IMPL_GET_NET_DEMO_NAME(testFunc) { \
    const char* GetNetDemoName() { \
        static const char* demoName = #testFunc; \
        return demoName; \
    }
}

CLIENT_TEST_DEMO(testFunc) { \
    void NetDemoTest(unsigned short port, const char* host) { \
        (void) host; \
        printf("%s start\r\n", #testFunc); \
        printf("I will connect to %s:%d\r\n", host, port); \
        testFunc(host, port); \
        printf("%s done!\r\n", #testFunc); \
    } \
}
    IMPL_GET_NET_DEMO_NAME(testFunc)

SERVER_TEST_DEMO(testFunc) { \
    void NetDemoTest(unsigned short port, const char* host) { \
        (void) host; \
        printf("%s start\r\n", #testFunc); \
        printf("I will listen on :%d\r\n", port); \
        testFunc(port); \
        printf("%s done!\r\n", #testFunc); \
    } \
}
    IMPL_GET_NET_DEMO_NAME(testFunc)

#endif // NET_DEMO_COMMON_H
