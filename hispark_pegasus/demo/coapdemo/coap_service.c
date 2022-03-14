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

#include "coap.h"
#include "net.h"
#include "los_task.h"
#include "lwip/tcpip.h"
#include "lwip/udp.h"


#define TEST_IPV4 1
static u32_t g_coapTestTaskid = -1;
static int g_servRunning = 0;
static coap_context_t *g_servCtx = NULL;
/*
 * The resource 'hello' GET method handler
 */
static void HelloHandler(coap_context_t *ctx, struct coap_resource_t *resource, coap_session_t *session,
                         coap_pdu_t *request, coap_binary_t *token, coap_string_t *query, coap_pdu_t *response)
{
    unsigned char buf[3];
    /* response with text "Hello World!" */
    const char* responseData = "Hello World! CoAP";
    size_t len = 0;
    unsigned char *data = NULL;
    (void)ctx;
    (void)resource;
    (void)session;
    response->code = COAP_RESPONSE_CODE(205); /* 返回值205，代表连接成功 */
    coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
                    coap_encode_var_safe(buf, 3, COAP_MEDIATYPE_TEXT_PLAIN), buf); /* 3个字节长度 */
    coap_add_data(response, strlen(responseData), (unsigned char *)responseData);
    if (coap_get_data(request, &len, &data)) {
        printf("Hello World! CoAP\n");
    }
}
void CoapServerThread(UINT32 uwParam1, UINT32 uwParam2, UINT32 uwParam3, UINT32 uwParam4)
{
    coap_context_t* ctx;
    (void)uwParam2;
    (void)uwParam3;
    (void)uwParam4;
    printf("[%s][%d] thread running\n", __FUNCTION__, __LINE__);
    ctx = (coap_context_t*)uwParam1;
    while (g_servRunning == 1) {
        hi_sleep(1000); /* 休眠1000ms */
        coap_check_notify_lwip(ctx);
    }
    if (g_servCtx != NULL) {
        coap_free_context_lwip(g_servCtx);
        g_servCtx = NULL;
    }
    printf("[%s][%d] thread exit\n", __FUNCTION__, __LINE__);
    return;
}
int CoapServerStart(void)
{
    TSK_INIT_PARAM_S stappTask;
    UINT32 ret;
    coap_address_t serv_addr;
    coap_resource_t* hello_resource;
    if (g_servRunning == 1) {
        return 0;
    }
    g_servRunning = 1;
    /* Prepare the CoAP server socket */
    coap_address_init(&serv_addr);
#if TEST_IPV4
#else
    ip_addr_set_any(true, &(serv_addr.addr));
#endif
    serv_addr.port = COAP_DEFAULT_PORT;
    g_servCtx = coap_new_context_lwip(&serv_addr);
    if (!g_servCtx) {
        return -1;
    }
    /* Initialize the hello resource */
    hello_resource = coap_resource_init(coap_make_str_const("hello"), 0);
    coap_register_handler(hello_resource, COAP_REQUEST_GET, HelloHandler);
    coap_add_resource(g_servCtx, hello_resource);
    /* create a thread task */
    stappTask.pfnTaskEntry = (TSK_ENTRY_FUNC)CoapServerThread;
    stappTask.uwStackSize = 10 * LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE; /* task为10 */
    stappTask.pcName = "coap_test_task";
    stappTask.usTaskPrio = 11; /* task为11 */
    stappTask.uwResved = LOS_TASK_STATUS_DETACHED;
    stappTask.auwArgs[0] = (UINT32)g_servCtx;
    printf("task create CoapServerThread\n");
    ret = LOS_TaskCreate(&g_coapTestTaskid, &stappTask);
    if (ret != 0) {
        dprintf("CoapServerThread create failed ! \n");
        return -1;
    }
    return 0;
}
void CoapServerStop(void)
{
    if (g_servRunning == 0) {
        printf("[%s][%d] not running\n", __FUNCTION__, __LINE__);
        return;
    }
    if (g_servCtx != NULL) {
        coap_free_context_lwip(g_servCtx);
        g_servCtx = NULL;
    }
    g_servRunning = 0;
    printf("[%s][%d] stopped\n", __FUNCTION__, __LINE__);
    return;
}
void NetDemoTest(void)
{
    printf("coapserviceTest start\r\n");
    CoapServerStart();
}