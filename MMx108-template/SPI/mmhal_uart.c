/**
 * Copyright 2024 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 * @file
 * This File implements platform specific shims for accessing the data-link transport.
 */

#include "mmhal_uart.h"
#include "mmutils.h"

__attribute__((weak)) void mmhal_uart_init(mmhal_uart_rx_cb_t rx_cb, void *rx_cb_arg)
{
    MM_UNUSED(rx_cb);
    MM_UNUSED(rx_cb_arg);
}

__attribute__((weak)) void mmhal_uart_deinit(void)
{
}

__attribute__((weak)) void mmhal_uart_tx(const uint8_t *tx_data, size_t length)
{
    MM_UNUSED(tx_data);
    MM_UNUSED(length);
}

__attribute__((weak)) bool mmhal_uart_set_deep_sleep_mode(enum mmhal_uart_deep_sleep_mode mode)
{
    return false;
}
