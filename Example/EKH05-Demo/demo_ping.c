/*
 * Copyright 2024 Morse Micro
 *
 * This file is licensed under terms that can be found in the LICENSE.md file in
 * the root directory of the Morse Micro IoT SDK software package.
 */

#include "demo_ping.h"
#include <stdarg.h>

#define TEMP_BUFFER_SIZE 128

#define PING_TARGET "192.168.1.1"
#define PING_COUNT 10

#ifndef PING_DATA_SIZE
/** Size of the ping request data, excluding 8-byte ICMP header. */
#define PING_DATA_SIZE 56
#endif
#ifndef PING_INTERVAL_MS
/** Interval between successive ping requests. */
#define PING_INTERVAL_MS 1000
#endif


/* http_terminal_buffer is a shared buffer to hold the output of ping task, so http can read it
 * from.*/
SharedBuffer http_terminal_buffer;

static bool break_ping = false;
static bool ping_in_progress = false;

static struct mmping_args args = MMPING_ARGS_DEFAULT;
static char local_output_str[TEMP_BUFFER_SIZE];

static struct mmosal_semb *ping_task_start = NULL;
static struct mmosal_task *ping_task_p;

/* dual_print prints into uart and http console. return false if the http console buffer can't take
 * more data.*/
bool dual_print(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(local_output_str, TEMP_BUFFER_SIZE, format, args);
    va_end(args);
    /* Print on uart console and to http console.*/
    printf("%s", local_output_str);
    return shared_buffer_append(&http_terminal_buffer, local_output_str);
}

/* ping_routine is the actual ping routine that pings a target.*/
void ping_routine(void)
{
    /** Executes the ping. */
    bool shared_buffer_ok=true;

    enum mmipal_status status = mmipal_get_local_addr(args.ping_src, args.ping_target);
    if (status != MMIPAL_SUCCESS)
    {
        printf("Failed to get local address for PING\n");
    }

    mmping_start(&args);
    shared_buffer_ok &= dual_print("\nPing %s %lu(%lu) bytes of data.\n", args.ping_target,
                                  args.ping_size, (MMPING_ICMP_ECHO_HDR_LEN + args.ping_size));
    mmosal_task_sleep(args.ping_interval_ms);

    struct mmping_stats stats;
    unsigned last_ping_total_count = 0;
    unsigned last_ping_recv_count = 0;

    mmping_stats(&stats);
    while (stats.ping_is_running)
    {
        mmping_stats(&stats);
        mmosal_task_sleep(args.ping_interval_ms / 2);
        if (stats.ping_total_count != last_ping_total_count ||
            stats.ping_recv_count != last_ping_recv_count)
        {
            shared_buffer_ok &= dual_print(
                "transmitted/received = %lu/%lu, round-trip min/avg/max = %lu/%lu/%lu ms\n",
                stats.ping_total_count, stats.ping_recv_count, stats.ping_min_time_ms,
                stats.ping_avg_time_ms, stats.ping_max_time_ms);

            if (break_ping || !shared_buffer_ok)
            {
                /* stop at the mid-point */
                break_ping = false;
                mmping_stop();
                break;
            }
        }
        last_ping_recv_count = stats.ping_recv_count;
        last_ping_total_count = stats.ping_total_count;
    }

    if (break_ping)
    {
        dual_print("Terminated by user.");
    }

    uint32_t loss = 0;
    if (stats.ping_total_count == 0)
    {
        loss = 0;
    }
    else
    {
        loss = (1000 * (stats.ping_total_count - stats.ping_recv_count) * 100 /
                stats.ping_total_count);
    }

    dual_print("\n--- %s ping statistics ---\n%lu packets transmitted, %lu packets received, ",
               stats.ping_receiver, stats.ping_total_count, stats.ping_recv_count);

    dual_print("%lu.%03lu%% packet loss round-trip min/avg/max = %lu/%lu/%lu ms\n", loss / 1000,
               loss % 1000, stats.ping_min_time_ms, stats.ping_avg_time_ms, stats.ping_max_time_ms);

    break_ping = false;
}

void demo_ping_task(void *arg)
{
    MM_UNUSED(arg);
    while (1)
    {
        /* Should wait forever until the binary is 1 */
        mmosal_semb_wait(ping_task_start, UINT32_MAX);
        ping_in_progress = true;
        ping_routine();
        ping_in_progress = false;
    }
}

void ping_init(void)
{
    MMOSAL_ASSERT(ping_task_start == NULL);
    ping_task_start = mmosal_semb_create("ping_task_start");

    /* First Use the default values, */
    strncpy(args.ping_target, PING_TARGET, sizeof(args.ping_target));
    args.ping_count = PING_COUNT;
    /* Then update them from configs if they exist.*/
    mmconfig_read_string("ping.target", args.ping_target, sizeof(args.ping_target));
    mmconfig_read_uint32("ping.count", &args.ping_count);

    ping_task_p = mmosal_task_create(demo_ping_task, NULL, MMOSAL_TASK_PRI_LOW, 1024, "ping");
    if (ping_task_p == NULL)
    {
        printf("Unable to start ping task\n\r");
    }
}

bool ping_get_in_progress(void) { return ping_in_progress; }

const char *ping_get_target(void) { return args.ping_target; }

uint32_t ping_get_count(void) { return args.ping_count; }

void ping_set_IP(const char *ip) { strncpy(args.ping_target, ip, sizeof(args.ping_target)); }

void ping_set_count(uint32_t count) { args.ping_count = count; }

void ping_stop(void) { break_ping = true; }

void ping_start(void)
{
    shared_buffer_reset(&http_terminal_buffer);

    if (ping_task_start != NULL)
    {
        /* start = 1 which trigger the ping */
        mmosal_semb_give(ping_task_start);
    }
}
