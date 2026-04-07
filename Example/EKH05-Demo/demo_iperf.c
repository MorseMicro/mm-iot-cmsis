/*
 * Copyright 2025-2026 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "demo_iperf.h"
#include "shared_buffer.h"
#include "mmiperf.h"


extern SharedBuffer http_terminal_buffer;
static struct mmosal_task *iperf_task_p;

bool is_tcp_server_up=false;
bool is_udp_server_up=false;
bool is_tcp_client_running=false;
bool is_udp_client_running=false;

const char *report_type_string(enum mmiperf_report_type type)
{
    switch (type)
    {
    case MMIPERF_TCP_DONE_CLIENT:
        return "TCP_CLIENT";
        break;
    case MMIPERF_UDP_DONE_CLIENT:
        return "UDP_CLIENT";
        break;
    case MMIPERF_TCP_DONE_SERVER:
        return "TCP_SERVER";
        break;
    case MMIPERF_UDP_DONE_SERVER:
        return "UDP_SERVER";
        break;
    default:
        return "";
        break;
    }
}
static const char units[] = {' ', 'K', 'M', 'G', 'T'};
/**
 * Function to format a given number of bytes into an appropriate SI base. I.e if you give it 1400
 * it will return 1 with unit_index set to 1 for Kilo.
 *
 * @warning This uses power of 10 units (kilo, mega, giga, etc). Not to be confused with power of 2
 *          units (kibi, mebi, gibi, etc).
 *
 * @param[in]   bytes       Original number of bytes
 * @param[out]  unit_index  Index into the @ref units array. Must not be NULL
 *
 * @return Number of bytes formatted to the appropriate unit given by the unit index.
 */
static uint32_t format_bytes(uint64_t bytes, uint8_t *unit_index)
{
    MMOSAL_ASSERT(unit_index != NULL);
    *unit_index = 0;

    while (bytes >= 1000 && *unit_index < 4)
    {
        bytes /= 1000;
        (*unit_index)++;
    }

    return bytes;
}

/**
 * Handle a report at the end of an iperf transfer.
 *
 * @param report    The iperf report.
 * @param arg       Opaque argument specified when iperf was started.
 * @param handle    The iperf instance handle returned when iperf was started.
 */
static void iperf_report_handler(const struct mmiperf_report *report, void *arg,
                                 mmiperf_handle_t handle)
{
    (void)arg;
    (void)handle;

    uint8_t bytes_transferred_unit_index = 0;
    uint32_t bytes_transferred_formatted = format_bytes(report->bytes_transferred,
                                                        &bytes_transferred_unit_index);

    dual_print(&http_terminal_buffer,"\nIperf Report\n");
    dual_print(&http_terminal_buffer,"  Report Type: %s\n",report_type_string(report->report_type));
    dual_print(&http_terminal_buffer,"  Remote Address: %s:%d\n", report->remote_addr, report->remote_port);
    dual_print(&http_terminal_buffer,"  Local Address:  %s:%d\n", report->local_addr, report->local_port);
    dual_print(&http_terminal_buffer,"  Transferred: %lu %cBytes, duration: %lu ms, bandwidth: %lu kbps\n",
           bytes_transferred_formatted, units[bytes_transferred_unit_index],
           report->duration_ms, report->bandwidth_kbitpsec);
    dual_print(&http_terminal_buffer,"\n");

    if ((report->report_type == MMIPERF_UDP_DONE_SERVER) ||
        (report->report_type == MMIPERF_TCP_DONE_SERVER))
    {
        dual_print(&http_terminal_buffer, "Waiting for client to connect...\n");
    }
    else if (report->report_type == MMIPERF_UDP_DONE_CLIENT)
    {
        is_udp_client_running = false;
    }
    else if (report->report_type == MMIPERF_TCP_DONE_CLIENT)
    {
        is_tcp_client_running = false;
    }
}

static void iperf_start_udp_server(uint16_t port) {
	struct mmiperf_server_args args = MMIPERF_SERVER_ARGS_DEFAULT;
	if (is_udp_server_up)
		return;
	args.local_port = port;
	args.report_fn = iperf_report_handler;
	mmiperf_handle_t iperf_handle = mmiperf_start_udp_server(&args);

	if (iperf_handle == NULL) {
		dual_print(&http_terminal_buffer,
				"Iperf UDP Server - Failed to get local address\n");
		return;
	}
	is_udp_server_up = true;


	dual_print(&http_terminal_buffer, "Started Iperf UDP Server - Execute cmd on AP:\n");

	struct mmipal_ip_config ip_config;
	enum mmipal_status ip_status = mmipal_get_ip_config(&ip_config);
	if (ip_status == MMIPAL_SUCCESS) {
		dual_print(&http_terminal_buffer,
				"IPv4: iperf -c %s -p %u -i 1 -u -b 20M\n",
				ip_config.ip_addr, args.local_port);
	}

	// TODO - Reinstate IPv6 prompt once it is working
	//	struct mmipal_ip6_config ip6_config;
	//	enum mmipal_status ip6_status = mmipal_get_ip6_config(&ip6_config);
	//	if (ip6_status == MMIPAL_SUCCESS) {
	//		dual_print(&http_terminal_buffer,
	//				"IPv6: iperf -c %s%wlan0 -p %u -i 1 -V -u -b 20M\n",
	//				ip6_config.ip6_addr[0], args.local_port);
	//	}
	dual_print(&http_terminal_buffer, "waiting for client to connect...\n");
}

static void iperf_start_tcp_server(uint16_t port) {
	struct mmiperf_server_args args = MMIPERF_SERVER_ARGS_DEFAULT;
	if (is_tcp_server_up)
		return;
	args.local_port = port;
	args.report_fn = iperf_report_handler;
	mmiperf_handle_t iperf_handle = mmiperf_start_tcp_server(&args);
	if (iperf_handle == NULL) {
		dual_print(&http_terminal_buffer,
				"Iperf TCP Server - Failed to get local address\n");
		return;
	}
	is_tcp_server_up = true;


	dual_print(&http_terminal_buffer, "Started Iperf TCP Server - Execute cmd on AP:\n");
	struct mmipal_ip_config ip_config;
	enum mmipal_status ip_status = mmipal_get_ip_config(&ip_config);
	if (ip_status == MMIPAL_SUCCESS) {
		dual_print(&http_terminal_buffer,
				"IPv4: iperf -c %s -p %u -i 1\n",
				ip_config.ip_addr, args.local_port);
	}

	// TODO - Reinstate IPv6 prompt once it is working
	//	struct mmipal_ip6_config ip6_config;
	//	enum mmipal_status ip6_status = mmipal_get_ip6_config(&ip6_config);
	//	if (ip6_status == MMIPAL_SUCCESS) {
	//		dual_print(&http_terminal_buffer,
	//				"IPv6: iperf -c %s%%wlan0 -p %u -i 1 -V\n",
	//				ip6_config.ip6_addr[0], args.local_port);
	//	}
	dual_print(&http_terminal_buffer, "waiting for client to connect...\n");
}

static void iperf_start_udp_client(const char *target_ip, uint16_t port, int amount)
{
    struct mmiperf_client_args args = MMIPERF_CLIENT_ARGS_DEFAULT;
    if (is_udp_client_running)
        return;
    strncpy(args.server_addr,target_ip,sizeof(args.server_addr));
    args.server_port = port;
    args.amount = amount;
    args.report_fn = iperf_report_handler;
    mmiperf_start_udp_client(&args);
    is_udp_client_running = true;
    dual_print(&http_terminal_buffer, "Started Iperf UDP client\n");
}

static void iperf_start_tcp_client(const char *target_ip, uint16_t port, uint32_t amount)
{
    struct mmiperf_client_args args = MMIPERF_CLIENT_ARGS_DEFAULT;
    if (is_tcp_client_running)
        return;
    strncpy(args.server_addr, target_ip, sizeof(args.server_addr));
    args.server_port = port;
    args.amount      = amount;
    args.report_fn   = iperf_report_handler;
    mmiperf_start_tcp_client(&args);
    is_tcp_client_running = true;
    dual_print(&http_terminal_buffer, "Started Iperf TCP client\n");
}

bool iperf_is_client_in_progress(void) { return is_tcp_client_running || is_udp_client_running; }
bool iperf_is_tcp_server_up(void) { return is_tcp_server_up; }
bool iperf_is_udp_server_up(void) { return is_udp_server_up; }

static struct mmosal_queue * iperf_command_pool_queue = NULL;

static void demo_iperf_task(void *arg)
{
    MM_UNUSED(arg);
    iperf_cmd_args_t args;
    while (1)
    {
        /*block here until a new command is pushed.*/
        if (!mmosal_queue_pop(iperf_command_pool_queue, &args, UINT32_MAX))
            continue;
        switch (args.mode)
        {
        case IPERF_MODE_NONE:
            printf("ERROR: abort is not supported.");
            break;
        case IPERF_MODE_UDP_SERVER:
            iperf_start_udp_server(args.port);
            break;
        case IPERF_MODE_TCP_SERVER:
            iperf_start_tcp_server(args.port);
            break;
        case IPERF_MODE_UDP_CLIENT:
            iperf_start_udp_client(args.target_ip, args.port, args.amount);
            break;
        case IPERF_MODE_TCP_CLIENT:
            iperf_start_tcp_client(args.target_ip, args.port, args.amount);
            break;
        default:
            printf("ERROR: incorrect iperf mode.");
            break;
        }
    }
}

void iperf_init(void)
{
    MMOSAL_ASSERT(iperf_task_p == NULL);
    MMOSAL_ASSERT(iperf_command_pool_queue == NULL);

    if (iperf_command_pool_queue == NULL)
    {
        iperf_command_pool_queue =
            mmosal_queue_create(2, sizeof(iperf_cmd_args_t), "iperf_cmd_pool_q");
    }

    iperf_task_p = mmosal_task_create(demo_iperf_task, NULL, MMOSAL_TASK_PRI_MIN, 1024, "iperf");
    if (iperf_task_p == NULL)
    {
        printf("Unable to start iperf task\n\r");
    }
}

bool iperf_push_cmd(iperf_cmd_args_t args)
{
    return mmosal_queue_push(iperf_command_pool_queue, &args, 100);
}
