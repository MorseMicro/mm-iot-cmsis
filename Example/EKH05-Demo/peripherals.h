/*
 * Copyright 2024 Morse Micro
 *
 * This file is licensed under terms that can be found in the LICENSE.md file in
 * the root directory of the Morse Micro IoT SDK software package.
 */

#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include <stdint.h>

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

#endif /* PERIPHERALS_H */