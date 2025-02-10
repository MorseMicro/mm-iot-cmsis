/*
 * Copyright (c) 2023 STMicroelectronics.
 * Copyright 2024 Morse Micro
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 */
#ifndef __W25Q16JV_H__
#define __W25Q16JV_H__
#include <stdint.h>
#include "stm32u5xx_hal_ospi.h"

uint8_t QSPI_WriteEnable(OSPI_HandleTypeDef *hospi);
uint8_t QSPI_EraseSector(OSPI_HandleTypeDef* hospi, uint32_t sector_address);
uint8_t QSPI_BlockSectorErase(OSPI_HandleTypeDef* hospi, uint32_t block_address);
uint8_t QSPI_ProgramPage(OSPI_HandleTypeDef* hospi, uint32_t start_address, uint8_t* data,
                      uint32_t length);
uint8_t QSPI_Read(OSPI_HandleTypeDef* hospi, uint32_t start_address, uint8_t* data, uint32_t length);
uint8_t QSPI_Configuration(OSPI_HandleTypeDef* hospi);
uint8_t QSPI_Init(OSPI_HandleTypeDef* hospi);
uint8_t QSPI_EnableMemoryMappedMode(OSPI_HandleTypeDef* hospi);
#endif /* __W25Q16JV_H__ */

