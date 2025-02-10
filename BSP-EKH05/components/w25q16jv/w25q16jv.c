/*
 * Copyright (c) 2023 STMicroelectronics.
 * Copyright 2024 Morse Micro
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 */
#include "stm32u5xx_hal.h"
#include "w25q16jv.h"
#include <stdio.h>
/*W25Q16JV memory parameters*/
#define MEMORY_FLASH_SIZE  0x200000 /* 16 MBits => 2,097,152 Bytes => 0x200000 */
#define MEMORY_BLOCK_SIZE  0x10000  /* 64KBytes */
#define MEMORY_SECTOR_SIZE 0x1000   /* 4kBytes */
#define MEMORY_PAGE_SIZE   0x100    /* 256Bytes */

/* Auto-polling values */
#define WRITE_ENABLE_MATCH_VALUE 0x02
#define WRITE_ENABLE_MASK_VALUE  0x02

#define MEMORY_READY_MATCH_VALUE 0x00
#define MEMORY_READY_MASK_VALUE  0x01

#define AUTO_POLLING_INTERVAL       0x20

/*W25Q16JV commands */
#define READ_ID_CMD               0xAB
#define JEDEC_ID_CMD              0x9F
#define READ_CMD                  0x03
#define FAST_READ_CMD             0x0B
#define FAST_READ_DUAL_OUT_CMD    0x3B
#define FAST_READ_QUAD_OUT_CMD    0x6B
#define FAST_READ_QUAD_IO_CMD     0xEB
#define FAST_READ_DUAL_IO_CMD     0xBB
#define WRITE_ENABLE_CMD          0x06
#define WRITE_DISABLE_CMD         0x04
#define READ_STATUS_REG_CMD       0x05
#define READ_STATUS_REG_2_CMD     0x35
#define WRITE_STATUS_REG_CMD      0x01
#define PAGE_PROG_CMD             0x02
#define DUAL_IN_FAST_PROG_CMD     0xA2
#define EXT_DUAL_IN_FAST_PROG_CMD 0xD2
#define QUAD_IN_FAST_PROG_CMD     0x32
#define EXT_QUAD_IN_FAST_PROG_CMD 0x12
#define SUBSECTOR_ERASE_CMD       0x20
#define BLOCK_64K_ERASE_CMD       0xD8
#define CHIP_ERASE_CMD            0xC7
#define RESET_ENABLE_CMD          0x66
#define RESET_MEMORY_CMD          0x99

static uint8_t OSPI_AutoPoll(OSPI_HandleTypeDef* hospi,uint32_t command, uint32_t mask, uint32_t match, uint32_t match_mode)
{
    OSPI_RegularCmdTypeDef sCommand;
    OSPI_AutoPollingTypeDef sConfig;

    /* Configure automatic polling mode to wait for write enabling ---- */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction = command;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.Address     = 0x0;
    sCommand.AddressMode = HAL_OSPI_ADDRESS_NONE;
    sCommand.DataMode    = HAL_OSPI_DATA_1_LINE;
    sCommand.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.NbData      = 2;
    sCommand.DummyCycles = 0;
    sCommand.DQSMode     = HAL_OSPI_DQS_DISABLE;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    sConfig.Match = match;
    sConfig.Mask = mask;
    sConfig.MatchMode = match_mode;
    sConfig.Interval = AUTO_POLLING_INTERVAL;
    sConfig.AutomaticStop = HAL_OSPI_AUTOMATIC_STOP_ENABLE;

    if (HAL_OSPI_AutoPolling(hospi, &sConfig, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }
    return HAL_OK;
}

static uint8_t OSPI_AutoPollMemoryReady(OSPI_HandleTypeDef *hospi)
{
    return OSPI_AutoPoll(hospi, READ_STATUS_REG_CMD, MEMORY_READY_MASK_VALUE,
                         MEMORY_READY_MATCH_VALUE, HAL_OSPI_MATCH_MODE_AND);
}

static uint8_t OSPI_AutoPollWriteEnable(OSPI_HandleTypeDef *hospi)
{
    return OSPI_AutoPoll(hospi, READ_STATUS_REG_CMD, WRITE_ENABLE_MATCH_VALUE,
                         WRITE_ENABLE_MASK_VALUE, HAL_OSPI_MATCH_MODE_AND);
}

uint8_t OSPI_ResetChip(OSPI_HandleTypeDef* hospi) {
    OSPI_RegularCmdTypeDef sCommand;

    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = RESET_ENABLE_CMD;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode = HAL_OSPI_DATA_NONE;
    sCommand.DummyCycles = 0;
    sCommand.DQSMode = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    sCommand.Instruction = RESET_MEMORY_CMD;
    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (OSPI_AutoPollMemoryReady(hospi) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

uint8_t QSPI_WriteEnable(OSPI_HandleTypeDef* hospi)
{
    OSPI_RegularCmdTypeDef sCommand;

    /* Enable write operations ------------------------------------------ */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = WRITE_ENABLE_CMD;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_NONE;
    sCommand.DummyCycles        = 0;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (OSPI_AutoPollWriteEnable(hospi) != HAL_OK)
    {
        return HAL_ERROR;
    }
    return HAL_OK;
}

uint8_t QSPI_EraseSector(OSPI_HandleTypeDef* hospi, uint32_t sector_address)
{
    OSPI_RegularCmdTypeDef sCommand;

    /* Enable write operations ------------------------------------------ */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = SUBSECTOR_ERASE_CMD;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_1_LINE;
    sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_NONE;
    sCommand.DummyCycles        = 0;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

    sCommand.Address = sector_address;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (OSPI_AutoPollMemoryReady(hospi) != HAL_OK)
    {
        return HAL_ERROR;
    }
    return HAL_OK;
}


uint8_t QSPI_BlockSectorErase(OSPI_HandleTypeDef* hospi, uint32_t block_address)
{
    OSPI_RegularCmdTypeDef sCommand;

    /* Enable write operations ------------------------------------------ */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = BLOCK_64K_ERASE_CMD;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_1_LINE;
    sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_NONE;
    sCommand.DummyCycles        = 0;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

    sCommand.Address = block_address;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (OSPI_AutoPollMemoryReady(hospi) != HAL_OK)
    {
        return HAL_ERROR;
    }
    return HAL_OK;
}

uint8_t QSPI_ProgramPage(OSPI_HandleTypeDef* hospi, uint32_t start_address, uint8_t* data,
                      uint32_t length)
{
    OSPI_RegularCmdTypeDef sCommand;

    /* Enable write operations ------------------------------------------ */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = QUAD_IN_FAST_PROG_CMD;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_1_LINE;
    sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_4_LINES;
    sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.NbData             = length;
    sCommand.DummyCycles        = 0;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

    sCommand.Address = start_address;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_OSPI_Transmit(hospi, data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }
    if (OSPI_AutoPollMemoryReady(hospi) != HAL_OK)
    {
        return HAL_ERROR;
    }
    return HAL_OK;
}

uint8_t QSPI_Read(OSPI_HandleTypeDef* hospi, uint32_t start_address, uint8_t* data,
                      uint32_t length)
{
    OSPI_RegularCmdTypeDef sCommand;

    /* Enable write operations ------------------------------------------ */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = FAST_READ_QUAD_OUT_CMD;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_1_LINE;
    sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_4_LINES;
    sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.NbData             = length;
    sCommand.DummyCycles        = 8;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

    sCommand.Address = start_address;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_OSPI_Receive(hospi, data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE))
    {
        return HAL_ERROR;
    }
    return HAL_OK;
}

uint8_t QSPI_Configuration(OSPI_HandleTypeDef* hospi) {
    OSPI_RegularCmdTypeDef sCommand;
    uint8_t regData[2];

    QSPI_WriteEnable(hospi);

    /* Enable write operations ------------------------------------------ */
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = WRITE_STATUS_REG_CMD;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_1_LINE;
    sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.NbData             = 2;
    sCommand.DummyCycles        = 0;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }
	regData[0] = 2;
	regData[1] = 2;

    if (HAL_OSPI_Transmit(hospi, regData, HAL_OSPI_TIMEOUT_DEFAULT_VALUE))
    {
        return HAL_ERROR;
    }

	return HAL_OK;
}

uint8_t QSPI_Init(OSPI_HandleTypeDef* hospi) {
    /* Prepare the QSPI flash start memorry-mapping. */

    if (OSPI_ResetChip(hospi) != HAL_OK) {
        return HAL_ERROR;
    }

    if (OSPI_AutoPollMemoryReady(hospi) != HAL_OK) {
        return HAL_ERROR;
    }

    if (QSPI_WriteEnable(hospi) != HAL_OK) {

        return HAL_ERROR;
    }

    if (QSPI_Configuration(hospi) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

uint8_t QSPI_EnableMemoryMappedMode(OSPI_HandleTypeDef* hospi)
{
    OSPI_RegularCmdTypeDef sCommand = {0};
    OSPI_MemoryMappedTypeDef sMemMappedCfg = {0};

    /* Memory-mapped mode configuration ------------------------------- */
    /* Common OSPI configuration*/
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_1_LINE;
    sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_4_LINES;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;

    /* Memory-mapped read configuration*/
    sCommand.OperationType = HAL_OSPI_OPTYPE_READ_CFG;
    sCommand.Instruction   = FAST_READ_QUAD_OUT_CMD;
    sCommand.DummyCycles   = 8;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* We are not inclined to use memory-mapped write, but the OSPI HAL lib mandates configuring
     * both actions.*/
    /* Memory-mapped write configuration*/
    sCommand.OperationType = HAL_OSPI_OPTYPE_WRITE_CFG;
    sCommand.Instruction   = QUAD_IN_FAST_PROG_CMD;
    sCommand.DummyCycles   = 0;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    sMemMappedCfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_DISABLE;

    if (HAL_OSPI_MemoryMapped(hospi, &sMemMappedCfg) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}
