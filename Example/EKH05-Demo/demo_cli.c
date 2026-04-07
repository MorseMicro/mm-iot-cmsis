/*
 * Copyright 2025 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mmconfig.h"
#include "mmhal_uart.h"
#include "mmutils.h"

#include "mmagic.h"
#include "mmregdb.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/mbedtls_config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif
#ifdef MBEDTLS_THREADING_ALT
#include "threading_alt.h"
#endif

#ifndef APPLICATION_VERSION
#error "APPLICATION_VERSION must be defined."
#endif /* APPLICATION_VERSION */

/** Flag to show if anything received on uart. used to disable autoconnect. */
bool uart_data_received = false;

/** Flag to show if connecting on power on. If true, will discard uart rx. */
bool auto_connect_in_progress = false;

/** Pointer to context for CLI receive callback. */
struct mmagic_cli *mmagic_cli_ctx;

bool cli_get_uart_data_received(void)
{
    return uart_data_received;
}

void cli_set_auto_connect_in_progress(bool in_progress)
{
    auto_connect_in_progress = in_progress;
}

/**
 * Handler for the UART receive callback.
 *
 * @param data      Received data.
 * @param length    Length of received data.
 * @param arg       Opaque argument (unused).
*/
void cli_uart_rx_handler(const uint8_t *data, size_t length, void *arg)
{
    MM_UNUSED(arg);
    if (mmagic_cli_ctx != NULL)
    {
        if(!auto_connect_in_progress)
        {
            uart_data_received = true;
            mmagic_cli_rx(mmagic_cli_ctx, (const char *)data, length);
        }
    }
}

/**
 * Handler for the CLI transmit callback.
 *
 * @param data      Data to transmit.
 * @param length    Length of data to transmit.
 * @param arg       Opaque argument (unused).
*/
void cli_tx_handler(const char *data, size_t length, void *arg)
{
    MM_UNUSED(arg);
    mmhal_uart_tx((const uint8_t *)data, length);
}

/**
 * Handler for the CLI set deep sleep mode callback.
 *
 * @param mode    The deep sleep mode to set.
 * @param arg       Opaque argument (unused).
 *
 * @returns true on success, false on failure.
*/
bool cli_set_deep_sleep_mode_handler(enum mmagic_deep_sleep_mode mode, void *arg)
{
    enum mmhal_uart_deep_sleep_mode mode_uart;

    MM_UNUSED(arg);

    switch (mode)
    {
    case MMAGIC_DEEP_SLEEP_MODE_DISABLED:
        mode_uart = MMHAL_UART_DEEP_SLEEP_DISABLED;
        break;

    case MMAGIC_DEEP_SLEEP_MODE_ONE_SHOT:
        mode_uart = MMHAL_UART_DEEP_SLEEP_ONE_SHOT;
        /* 1ms delay to flush out the LF that follows a CR - or else this will wake us up */
        mmosal_task_sleep(1);
        break;

    default:
        return false;
    }

    return mmhal_uart_set_deep_sleep_mode(mode_uart);
}


void cli_init(void)
{
    char cli_mode[4] = "cli";
    (void)mmconfig_read_string("cli.mode", cli_mode, sizeof(cli_mode));

    mmhal_uart_init(cli_uart_rx_handler, NULL);

    /* Initialize mbedTLS threading (required if MBEDTLS_THREADING_ALT is defined) */
#ifdef MBEDTLS_THREADING_ALT
    mbedtls_platform_threading_init();
#endif

    /* If the cli.mode config variable is set to m2m then use MMAGIC in binary machine-to-machine
     * mode, otherwise use it in interactive CLI mode. */
    if (strcasecmp(cli_mode, "m2m"))
    {
        const struct mmagic_cli_init_args init_args = {
            .app_version = APPLICATION_VERSION,
            .tx_cb = cli_tx_handler,
            .set_deep_sleep_mode_cb = cli_set_deep_sleep_mode_handler,
            .reg_db = get_regulatory_db(),
        };
        mmagic_cli_ctx = mmagic_cli_init(&init_args);
        printf("CLI interface enabled\n");
    }
    else
    {
        const struct mmagic_m2m_agent_init_args init_args = {
            .app_version = APPLICATION_VERSION,
            .reg_db = get_regulatory_db(),
        };
        struct mmagic_m2m_agent *mmagic_m2m_agent = mmagic_m2m_agent_init(&init_args);
        MM_UNUSED(mmagic_m2m_agent);
        printf("M2M interface enabled\n");
    }
}
