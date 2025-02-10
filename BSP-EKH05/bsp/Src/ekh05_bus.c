/**
  ******************************************************************************
  * @file    ekh05_bus.c
  * @author  MCD Application Team
  * @brief   This file provides a set of firmware functions to communicate
  *          with  external devices available on STM32U575I-EVAL board
  *          from STMicroelectronics.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * Copyright 2024 Morse Micro
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ekh05_bus.h"
#include "ekh05_errno.h"



/**
  * @brief  Write a value in a register of the device through BUS.
  * @param  DevAddr    Device address on Bus.
  * @param  Reg        The target register address to write
  * @param  MemAddSize Size of internal memory address
  * @param  pData      The target register value to be written
  * @param  Length     data length in bytes
  * @retval BSP status
  */
static int32_t I2C_WriteReg(uint16_t DevAddr, uint16_t Reg, uint16_t MemAddSize, uint8_t* pData,
                             uint16_t Length)
{
    if (HAL_I2C_Mem_Write(&hi2c1, DevAddr, Reg, MemAddSize, pData, Length, 10000) == HAL_OK)
    {
        return BSP_ERROR_NONE;
    }

    return BSP_ERROR_BUS_FAILURE;
}

/**
  * @brief  Read a register of the device through BUS
  * @param  DevAddr    Device address on BUS
  * @param  Reg        The target register address to read
  * @param  MemAddSize Size of internal memory address
  * @param  pData      The target register value to be written
  * @param  Length     data length in bytes
  * @retval BSP status
  */
static int32_t I2C_ReadReg(uint16_t DevAddr, uint16_t Reg, uint16_t MemAddSize, uint8_t* pData,
                            uint16_t Length)
{
    if (HAL_I2C_Mem_Read(&hi2c1, DevAddr, Reg, MemAddSize, pData, Length, 10000) == HAL_OK)
    {
        return BSP_ERROR_NONE;
    }

    return BSP_ERROR_BUS_FAILURE;
}

/**
  * @brief  Receive data from the device through BUS
  * @param  DevAddr    Device address on BUS
  * @param  pData      The target register value to be received
  * @param  Length     data length in bytes
  * @retval BSP status
  */
static int32_t I2C_Recv(uint16_t DevAddr, uint8_t* pData, uint16_t Length)
{
    if (HAL_I2C_Master_Receive(&hi2c1, DevAddr, pData, Length, 10000) == HAL_OK)
    {
        return BSP_ERROR_NONE;
    }

    return BSP_ERROR_BUS_FAILURE;
}

/**
  * @brief  Send data to the device through BUS
  * @param  DevAddr    Device address on BUS
  * @param  pData      The target register value to be sent
  * @param  Length     data length in bytes
  * @retval BSP status
  */
static int32_t I2C_Send(uint16_t DevAddr, uint8_t* pData, uint16_t Length)
{
    if (HAL_I2C_Master_Transmit(&hi2c1, DevAddr, pData, Length, 10000) == HAL_OK)
    {
        return BSP_ERROR_NONE;
    }

    return BSP_ERROR_BUS_FAILURE;
}

int32_t BSP_I2C_Init(void) { return 0; }

int32_t BSP_I2C_DeInit(void) { return 0; }

/**
  * @brief  Write a 8bit value in a register of the device through BUS.
  * @param  DevAddr Device address on Bus.
  * @param  Reg    The target register address to write
  * @param  pData  The target register value to be written
  * @param  Length buffer size to be written
  * @retval BSP status
  */
int32_t BSP_I2C_WriteReg(uint16_t DevAddr, uint16_t Reg, uint8_t* pData, uint16_t Length)
{
    int32_t ret;

#if defined(BSP_USE_CMSIS_OS)
    /* Get semaphore to prevent multiple I2C access */
    osSemaphoreWait(BspI2cSemaphore, osWaitForever);
#endif /* BSP_USE_CMSIS_OS */
    if (I2C_WriteReg(DevAddr, Reg, I2C_MEMADD_SIZE_8BIT, pData, Length) == 0)
    {
        ret = BSP_ERROR_NONE;
    }
    else
    {
        if (HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF)
        {
            ret = BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE;
        }
        else
        {
            ret = BSP_ERROR_PERIPH_FAILURE;
        }
    }
#if defined(BSP_USE_CMSIS_OS)
    /* Release semaphore to prevent multiple I2C access */
    osSemaphoreRelease(BspI2cSemaphore);
#endif /* BSP_USE_CMSIS_OS */

    return ret;
}

/**
  * @brief  Read a 8bit register of the device through BUS
  * @param  DevAddr Device address on BUS
  * @param  Reg     The target register address to read
  * @param  pData   Pointer to data buffer
  * @param  Length  Length of the data
  * @retval BSP status
  */
int32_t BSP_I2C_ReadReg(uint16_t DevAddr, uint16_t Reg, uint8_t* pData, uint16_t Length)
{
    int32_t ret;

#if defined(BSP_USE_CMSIS_OS)
    /* Get semaphore to prevent multiple I2C access */
    osSemaphoreWait(BspI2cSemaphore, osWaitForever);
#endif /* BSP_USE_CMSIS_OS */
    if (I2C_ReadReg(DevAddr, Reg, I2C_MEMADD_SIZE_8BIT, pData, Length) == 0)
    {
        ret = BSP_ERROR_NONE;
    }
    else
    {
        if (HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF)
        {
            ret = BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE;
        }
        else
        {
            ret = BSP_ERROR_PERIPH_FAILURE;
        }
    }
#if defined(BSP_USE_CMSIS_OS)
    /* Release semaphore to prevent multiple I2C access */
    osSemaphoreRelease(BspI2cSemaphore);
#endif /* BSP_USE_CMSIS_OS */

    return ret;
}

/**
  * @brief  Write a 16bit value in a register of the device through BUS.
  * @param  DevAddr Device address on Bus.
  * @param  Reg    The target register address to write
  * @param  pData  The target register value to be written
  * @param  Length buffer size to be written
  * @retval BSP status
  */
int32_t BSP_I2C_WriteReg16(uint16_t DevAddr, uint16_t Reg, uint8_t* pData, uint16_t Length)
{
    int32_t ret;

#if defined(BSP_USE_CMSIS_OS)
    /* Get semaphore to prevent multiple I2C access */
    osSemaphoreWait(BspI2cSemaphore, osWaitForever);
#endif /* BSP_USE_CMSIS_OS */
    if (I2C_WriteReg(DevAddr, Reg, I2C_MEMADD_SIZE_16BIT, pData, Length) == 0)
    {
        ret = BSP_ERROR_NONE;
    }
    else
    {
        if (HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF)
        {
            ret = BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE;
        }
        else
        {
            ret = BSP_ERROR_PERIPH_FAILURE;
        }
    }
#if defined(BSP_USE_CMSIS_OS)
    /* Release semaphore to prevent multiple I2C access */
    osSemaphoreRelease(BspI2cSemaphore);
#endif /* BSP_USE_CMSIS_OS */

    return ret;
}

/**
  * @brief  Read a 16bit register of the device through BUS
  * @param  DevAddr Device address on BUS
  * @param  Reg     The target register address to read
  * @param  pData   Pointer to data buffer
  * @param  Length  Length of the data
  * @retval BSP status
  */
int32_t BSP_I2C_ReadReg16(uint16_t DevAddr, uint16_t Reg, uint8_t* pData, uint16_t Length)
{
    int32_t ret;

#if defined(BSP_USE_CMSIS_OS)
    /* Get semaphore to prevent multiple I2C access */
    osSemaphoreWait(BspI2cSemaphore, osWaitForever);
#endif /* BSP_USE_CMSIS_OS */
    if (I2C_ReadReg(DevAddr, Reg, I2C_MEMADD_SIZE_16BIT, pData, Length) == 0)
    {
        ret = BSP_ERROR_NONE;
    }
    else
    {
        if (HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF)
        {
            ret = BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE;
        }
        else
        {
            ret = BSP_ERROR_PERIPH_FAILURE;
        }
    }
#if defined(BSP_USE_CMSIS_OS)
    /* Release semaphore to prevent multiple I2C access */
    osSemaphoreRelease(BspI2cSemaphore);
#endif /* BSP_USE_CMSIS_OS */

    return ret;
}

/**
  * @brief  Read data
  * @param  DevAddr Device address on BUS
  * @param  pData   Pointer to data buffer
  * @param  Length  Length of the data
  * @retval BSP status
  */
int32_t BSP_I2C_Recv(uint16_t DevAddr, uint8_t* pData, uint16_t Length)
{
    int32_t ret;

#if defined(BSP_USE_CMSIS_OS)
    /* Get semaphore to prevent multiple I2C access */
    osSemaphoreWait(BspI2cSemaphore, osWaitForever);
#endif /* BSP_USE_CMSIS_OS */
    if (I2C_Recv(DevAddr, pData, Length) == 0)
    {
        ret = BSP_ERROR_NONE;
    }
    else
    {
        if (HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF)
        {
            ret = BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE;
        }
        else
        {
            ret = BSP_ERROR_PERIPH_FAILURE;
        }
    }
#if defined(BSP_USE_CMSIS_OS)
    /* Release semaphore to prevent multiple I2C access */
    osSemaphoreRelease(BspI2cSemaphore);
#endif /* BSP_USE_CMSIS_OS */

    return ret;
}

/**
  * @brief  Send data
  * @param  DevAddr Device address on BUS
  * @param  pData   Pointer to data buffer
  * @param  Length  Length of the data
  * @retval BSP status
  */
int32_t BSP_I2C_Send(uint16_t DevAddr, uint8_t* pData, uint16_t Length)
{
    int32_t ret;

#if defined(BSP_USE_CMSIS_OS)
    /* Get semaphore to prevent multiple I2C access */
    osSemaphoreWait(BspI2cSemaphore, osWaitForever);
#endif /* BSP_USE_CMSIS_OS */
    if (I2C_Send(DevAddr, pData, Length) == 0)
    {
        ret = BSP_ERROR_NONE;
    }
    else
    {
        if (HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF)
        {
            ret = BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE;
        }
        else
        {
            ret = BSP_ERROR_PERIPH_FAILURE;
        }
    }
#if defined(BSP_USE_CMSIS_OS)
    /* Release semaphore to prevent multiple I2C access */
    osSemaphoreRelease(BspI2cSemaphore);
#endif /* BSP_USE_CMSIS_OS */

    return ret;
}


/**
  * @brief  Checks if target device is ready for communication.
  * @note   This function is used with Memory devices
  * @param  DevAddr  Target device address
  * @param  Trials      Number of trials
  * @retval BSP status
  */
int32_t BSP_I2C_IsReady(uint16_t DevAddr, uint32_t Trials){
  int32_t ret = BSP_ERROR_NONE;

#if defined(BSP_USE_CMSIS_OS)
  /* Get semaphore to prevent multiple I2C access */
  osSemaphoreWait(BspI2cSemaphore, osWaitForever);
#endif /* BSP_USE_CMSIS_OS */
  if (HAL_I2C_IsDeviceReady(&hi2c1, DevAddr, Trials, 1000) != HAL_OK)
  {
    ret = BSP_ERROR_BUSY;
  }
#if defined(BSP_USE_CMSIS_OS)
  /* Release semaphore to prevent multiple I2C access */
  osSemaphoreRelease(BspI2cSemaphore);
#endif /* BSP_USE_CMSIS_OS */

  return ret;
}
