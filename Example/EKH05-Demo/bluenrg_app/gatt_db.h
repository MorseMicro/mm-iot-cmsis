/**
  ******************************************************************************
  * @file    App/gatt_db.h
  * @author  SRA Application Team
  * @brief   Header file for App/gatt_db.c
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef GATT_DB_H
#define GATT_DB_H

/* Includes ------------------------------------------------------------------*/
#include "hci.h"
#include "demo_accelerometer.h"
#include "demo_temperature.h"

/* Exported defines ----------------------------------------------------------*/
#define X_OFFSET 200
#define Y_OFFSET 50
#define Z_OFFSET 1000

/**
 * @brief Number of application services
 */
#define NUMBER_OF_APPLICATION_SERVICES (2)


/* Exported typedef ----------------------------------------------------------*/

/* Exported function prototypes ----------------------------------------------*/
tBleStatus Add_HWServW2ST_Service(void);
tBleStatus Add_SWServW2ST_Service(void);
void Read_Request_CB(uint16_t handle);
void Attribute_Modified_Request_CB(uint16_t Connection_Handle, uint16_t attr_handle,
                                   uint16_t Offset, uint8_t data_length, uint8_t *att_data);
tBleStatus Environmental_Update(th_value_t th);
tBleStatus Acc_Update(accelerometer_value_t x_axes);
void update_ble_ip_gw(const char *ip, const char *gw);

#endif /* GATT_DB_H */
