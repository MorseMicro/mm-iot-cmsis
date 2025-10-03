/*
 * Copyright 2025 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DEMO_IPERF__H__
#define __DEMO_IPERF__H__

#include <string.h>
#include <stdlib.h>

#include "mmutils.h"
#include "mmosal.h"
#include "mmconfig.h"
#include "mmping.h"
#include "mmipal.h"
#include "tcpip.h"

typedef enum
{
    IPERF_MODE_NONE       = 0,
    IPERF_MODE_UDP_SERVER = 1,
    IPERF_MODE_TCP_SERVER = 2,
    IPERF_MODE_UDP_CLIENT = 3,
    IPERF_MODE_TCP_CLIENT = 4
} iperf_mode_t;

typedef struct
{
    iperf_mode_t mode;
    uint16_t port;
    int amount;               /*Only needed for client mode*/
    char target_ip[4 * 3 + 3 + 1]; /*XXX.XXX.XXX.XXX and a NULL. Only needed for client mode*/
} iperf_cmd_args_t;

void iperf_abort(void);
bool iperf_push_cmd(iperf_cmd_args_t args);
void iperf_init(void);
bool iperf_is_client_in_progress(void);
bool iperf_is_tcp_server_up(void);
bool iperf_is_udp_server_up(void);

#endif
