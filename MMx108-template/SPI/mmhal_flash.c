/**
 * Copyright 2024 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 * @file
 * This File implements BSP specific shims for accessing the flash.
 */

#include <mmhal.h>

__attribute__((weak)) uint32_t mmhal_flash_getblocksize(uint32_t block_address)
{
    return 0;
}

__attribute__((weak)) int mmhal_flash_erase(uint32_t block_address)
{
    return -1;
}

__attribute__((weak)) int mmhal_flash_read(uint32_t read_address, uint8_t *buf, size_t size)
{
    return -1;
}

__attribute__((weak)) int mmhal_flash_write(uint32_t write_address, const uint8_t *data, size_t size)
{
    return -1;
}

__attribute__((weak)) const struct mmhal_flash_partition_config* mmhal_get_mmconfig_partition(void)
{
    return NULL;
}
