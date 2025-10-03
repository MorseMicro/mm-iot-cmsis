/*
 * Copyright 2021-2024 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mmhal.h"
#include "mmutils.h"

__attribute__((weak)) void mmhal_set_led(uint8_t led, uint8_t level)
{
    MM_UNUSED(led);
    MM_UNUSED(level);
}

__attribute__((weak)) void mmhal_set_error_led(bool state)
{
    MM_UNUSED(state);
}

__attribute__((weak)) enum mmhal_button_state mmhal_get_button(enum mmhal_button_id button_id)
{
    return BUTTON_RELEASED;
}

__attribute__((weak)) bool mmhal_set_button_callback(enum mmhal_button_id button_id,
                                mmhal_button_state_cb_t button_state_cb)
{
    return false;
}

__attribute__((weak)) mmhal_button_state_cb_t mmhal_get_button_callback(enum mmhal_button_id button_id)
{
    return NULL;
}

__attribute__((weak)) time_t mmhal_get_time()
{
    return 0;
}

__attribute__((weak)) void mmhal_set_time(time_t epoch)
{
    MM_UNUSED(epoch);
}

__attribute__((weak)) void mmhal_reset(void)
{
}

__attribute__((weak)) void mmhal_set_debug_pins(uint32_t mask, uint32_t values)
{
    MM_UNUSED(mask);
    MM_UNUSED(values);
}

__attribute__((weak)) bool mmhal_get_hardware_version(char * version_buffer, size_t version_buffer_length)
{
    return false;
}
