/*
 * Copyright 2024 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include <stdint.h>
#include <stdbool.h>

#define SAVED_IMAGE_DATA_SIZE *(uint32_t *)OCTOSPI1_BASE

void periphs_start(void);
bool periphs_qspi_is_memmap_running(void);
bool periphs_qspi_flash_lock(void);
void periphs_qspi_flash_unlock(void);
bool periphs_jpeg_buffer_lock(void);
void periphs_jpeg_buffer_unlock(void);
uint8_t periphs_toggle_leds(void);
uint32_t periphs_get_live_jpeg_size(void);
const uint8_t *periphs_get_jpeg_buffer(void);
void periphs_set_country_code(const char *country_code);

#endif /* PERIPHERALS_H */