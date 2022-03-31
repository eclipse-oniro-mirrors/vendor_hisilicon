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
#include "coap_session_internal.h"
#include "iot_config.h"

#define TEST_IPV4 1
#define DHCP_COAP_TOKEN_LEN (4)
static coap_context_t* g_cliCtx = NULL;
/* The response handler */
static void MessageHandler(struct coap_context_t *ctx, coap_session_t *session, coap_pdu_t *sent,
    coap_pdu_t *received, coap_tid_t id)
{
    unsigned char* data;
    size_t dataLen;
    (void)ctx;
    (void)session;
    (void)sent;
    (void)received;
    (void)id;
    if (COAP_RESPONSE_CLASS(received->code) == 2) { /* 获取的数据2 */
        if (coap_get_data(received, &dataLen, &data)) {
            printf("Received data\n");
        }
    }
}

int CoapClientStart(void)
{
    coap_address_t src_addr;
    if (g_cliCtx != NULL) {
        return 0;
    }
    /* Prepare coap socket */
    coap_address_init(&src_addr);
#if TEST_IPV4
    ip_addr_set_any(false, &(src_addr.addr));
#else
    ip_addr_set_any(true, &(src_addr.addr));
#endif
    src_addr.port = 23456; /* 主机端口号23456 */
    g_cliCtx = coap_new_context_lwip(&src_addr);
    if (!g_cliCtx) {
        return -1;
    }
    /* Set the response handler */
    coap_register_response_handler(g_cliCtx, MessageHandler);
    return 0;
}
void CoapClientStop(void)
{
    if (g_cliCtx != NULL) {
        coap_free_context_lwip(g_cliCtx);
        g_cliCtx = NULL;
    }
    printf("[%s][%d] stopped\n", __FUNCTION__, __LINE__);
    return;
}
/* to create a new token value depanding on time */
s32_t CoapNewToken(u16_t msg_id, u8_t *token, u8_t token_len)
{
    u32_t now_ms;
    if ((token == NULL) || (token_len < DHCP_COAP_TOKEN_LEN)) {
        return -1;
    }
    now_ms = sys_now();
    token[0] = (u8_t)(msg_id); /* 0数据 */
    token[1] = (u8_t)(msg_id >> 8); /* 1数据，8数据 */
    token[2] = (u8_t)(now_ms); /* 2数据 */
    token[3] = (u8_t)(now_ms >> 8); /* 3数据，8数据 */
    return 0;
}
#define STRING_LEN (7)
int CoapClientSendMsg(char* dst)
{
    coap_address_t dst_addr, listen_addr;
    static coap_uri_t uri;
    coap_pdu_t* request;
    coap_session_t *session = NULL;
    char serverUri[128] = {0};
    u8_t temp_token[DHCP_COAP_TOKEN_LEN] = {0};
    unsigned char getMethod = COAP_REQUEST_GET;
    /* The destination endpoint */
    coap_address_init(&dst_addr);
    printf("[%s][%d] server : %s\n", __FUNCTION__, __LINE__, dst);

    if (!ipaddr_aton(dst, &(dst_addr.addr))) {
        return -1;
    }
    dst_addr.port = COAP_DEFAULT_PORT;
    /* try to reuse existed session */
    session = coap_session_get_by_peer(g_cliCtx, &dst_addr, 0);
    if (session == NULL) {
        coap_address_init(&listen_addr);
        ip_addr_set_any(false, &(listen_addr.addr));
        listen_addr.port = 23456; /* 监听端口号23456 */
        session = coap_new_client_session(g_cliCtx, &listen_addr, &dst_addr, COAP_PROTO_UDP);
        if (session == NULL) {
            printf("[%s][%d] new client session failed\n", __FUNCTION__, __LINE__);
            return -1;
        }
        session->sock.pcb = g_cliCtx->endpoint;
        SESSIONS_ADD(g_cliCtx->endpoint->sessions, session);
    }
 /* Prepare the request */
    strcpy_s(serverUri, STRING_LEN, "/hello");
    coap_split_uri((unsigned char *)serverUri, strlen(serverUri), &uri);
    request = coap_new_pdu(session);
    if (request == NULL) {
        printf("[%s][%d] get pdu failed\n", __FUNCTION__, __LINE__);
        return -1;
    }
    request->type = COAP_MESSAGE_CON;
    request->tid = coap_new_message_id(session);
    (void)CoapNewToken(request->tid, temp_token, DHCP_COAP_TOKEN_LEN);
    if (coap_add_token(request, DHCP_COAP_TOKEN_LEN, temp_token) == 0) {
        printf("[%s][%d] add token failed\n", __FUNCTION__, __LINE__);
    }
    request->code = getMethod;
    coap_add_option(request, COAP_OPTION_URI_PATH, uri.path.length, uri.path.s);
    char requestData[64] = {0};
    (void)snprintf_s(requestData, sizeof(requestData), sizeof(requestData)-1, "%s", "Hello coap");
    coap_add_data(request, 4 + strlen((const char *)(requestData + 4)), (unsigned char *)requestData);
    coap_send_lwip(session, request);
    return 0;
}

int CoapClientSend(void)
{
    if (g_cliCtx == NULL) {
        return -1;
    }
    /* argv[0] is server_addr */
    CoapClientSendMsg(PARAM_SERVER_ADDR); /* 主机IP地址 */
    return 0;
}

void NetDemoTest(void)
{
    printf("coapclientTest start\r\n");
    CoapClientStart();
    CoapClientSend();
}