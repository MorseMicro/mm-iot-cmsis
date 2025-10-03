/*
 * Copyright 2021-2024 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mmhal.h"
#include "mmutils.h"

__attribute__((weak)) void mmhal_early_init(void)
{
}

__attribute__((weak)) void mmhal_init(void)
{
}

__attribute__((weak)) enum mmhal_isr_state mmhal_get_isr_state(void)
{
    return MMHAL_NOT_IN_ISR;
}

__attribute__((weak)) void mmhal_log_write(const uint8_t *data, size_t length)
{
    MM_UNUSED(data);
    MM_UNUSED(length);
}

__attribute__((weak)) void mmhal_log_flush(void)
{
}

__attribute__((weak)) void mmhal_read_mac_addr(uint8_t *mac_addr)
{
}

__attribute__((weak)) uint32_t mmhal_random_u32(uint32_t min, uint32_t max)
{
    return 0;
}

__attribute__((weak)) void mmhal_set_deep_sleep_veto(uint8_t veto_id)
{
    MM_UNUSED(veto_id);
}

__attribute__((weak)) void mmhal_clear_deep_sleep_veto(uint8_t veto_id)
{
    MM_UNUSED(veto_id);
}

__attribute__((weak)) void mmhal_sleep_abort(enum mmhal_sleep_state sleep_state)
{
    MM_UNUSED(sleep_state);
}

__attribute__((weak)) enum mmhal_sleep_state mmhal_sleep_prepare(uint32_t expected_idle_time_ms)
{
    MM_UNUSED(expected_idle_time_ms);
    return MMHAL_SLEEP_DISABLED;
}

__attribute__((weak)) uint32_t mmhal_sleep(enum mmhal_sleep_state sleep_state, uint32_t expected_idle_time_ms)
{
    MM_UNUSED(expected_idle_time_ms);
    MM_UNUSED(sleep_state);
    return 0;
}

__attribute__((weak)) void mmhal_sleep_cleanup(void)
{
}