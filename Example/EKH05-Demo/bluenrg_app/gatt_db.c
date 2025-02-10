/**
  ******************************************************************************
  * @file    App/gatt_db.c
  * @author  SRA Application Team
  * @brief   Functions to build GATT DB and handle GATT events.
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

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "gatt_db.h"
#include "bluenrg1_aci.h"
#include "bluenrg1_hci_le.h"
#include "bluenrg1_gatt_aci.h"
#include "app_bluenrg_2.h"

/* Private macros ------------------------------------------------------------*/
/** @brief Macro that stores Value into a buffer in Little Endian Format (2 bytes)*/
#define HOST_TO_LE_16(buf, val)    ( ((buf)[0] =  (uint8_t) (val)    ) , \
                                   ((buf)[1] =  (uint8_t) (val>>8) ) )

/** @brief Macro that stores Value into a buffer in Little Endian Format (4 bytes) */
#define HOST_TO_LE_32(buf, val)    ( ((buf)[0] =  (uint8_t) (val)     ) , \
                                   ((buf)[1] =  (uint8_t) (val>>8)  ) , \
                                   ((buf)[2] =  (uint8_t) (val>>16) ) , \
                                   ((buf)[3] =  (uint8_t) (val>>24) ) )

#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
do {\
    uuid_struct[0] = uuid_0; uuid_struct[1] = uuid_1; uuid_struct[2] = uuid_2; uuid_struct[3] = uuid_3; \
        uuid_struct[4] = uuid_4; uuid_struct[5] = uuid_5; uuid_struct[6] = uuid_6; uuid_struct[7] = uuid_7; \
            uuid_struct[8] = uuid_8; uuid_struct[9] = uuid_9; uuid_struct[10] = uuid_10; uuid_struct[11] = uuid_11; \
                uuid_struct[12] = uuid_12; uuid_struct[13] = uuid_13; uuid_struct[14] = uuid_14; uuid_struct[15] = uuid_15; \
}while(0)

/* Hardware Characteristics Service */
#define COPY_HW_SENS_W2ST_SERVICE_UUID(uuid_struct)    COPY_UUID_128(uuid_struct,0x00,0x00,0x00,0x00,0x00,0x01,0x11,0xe1,0x9a,0xb4,0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_ENVIRONMENTAL_W2ST_CHAR_UUID(uuid_struct) COPY_UUID_128(uuid_struct,0x00,0x00,0x00,0x00,0x00,0x01,0x11,0xe1,0xac,0x36,0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_ACC_W2ST_CHAR_UUID(uuid_struct)           COPY_UUID_128(uuid_struct,0x00,0xE0,0x00,0x00,0x00,0x01,0x11,0xe1,0xac,0x36,0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_IP_W2ST_CHAR_UUID(uuid_struct)            COPY_UUID_128(uuid_struct,0x00,0xE0,0x00,0x00,0x00,0x01,0x11,0xe1,0xac,0xa2,0x00,0x02,0xa5,0xd5,0xc5,0x1b)

/* Private variables ---------------------------------------------------------*/
uint16_t HWServW2STHandle, EnvironmentalCharHandle, AccCharHandle, IpCharHandle;

/* UUIDS */
Service_UUID_t service_uuid;
Char_UUID_t char_uuid;

__IO uint8_t send_env;
__IO uint8_t send_mot;

extern __IO uint16_t connection_handle;


/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Add the 'HW' service (and the Environmental and AccGyr characteristics).
 * @param  None
 * @retval tBleStatus Status
 */
tBleStatus Add_HWServW2ST_Service(void)
{
  tBleStatus ret;
  uint8_t uuid[16];
  /* num of characteristics of this service */
  uint8_t char_number = 5;
  /* number of attribute records that can be added to this service */
  uint8_t max_attribute_records = 1+(3*char_number);

  /* add HW_SENS_W2ST service */
  COPY_HW_SENS_W2ST_SERVICE_UUID(uuid);
  BLUENRG_memcpy(&service_uuid.Service_UUID_128, uuid, 16);
  ret = aci_gatt_add_service(UUID_TYPE_128, &service_uuid, PRIMARY_SERVICE,
                             max_attribute_records, &HWServW2STHandle);
  if (ret != BLE_STATUS_SUCCESS)
    return BLE_STATUS_ERROR;

  /* Fill the Environmental BLE Characteristc */
  COPY_ENVIRONMENTAL_W2ST_CHAR_UUID(uuid);
  uuid[14] |= 0x04; /* One Temperature value*/
  uuid[14] |= 0x10; /* Pressure value*/
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(HWServW2STHandle, UUID_TYPE_128, &char_uuid,
                           64,
                           CHAR_PROP_NOTIFY|CHAR_PROP_READ,
                           ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 0, &EnvironmentalCharHandle);
  if (ret != BLE_STATUS_SUCCESS)
    return BLE_STATUS_ERROR;

  /* Fill the AccGyroMag BLE Characteristc */
  COPY_ACC_W2ST_CHAR_UUID(uuid);
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(HWServW2STHandle, UUID_TYPE_128, &char_uuid,
                           64,
                           CHAR_PROP_NOTIFY,
                           ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 0, &AccCharHandle);
  if (ret != BLE_STATUS_SUCCESS)
    return BLE_STATUS_ERROR;

  /* Fill the AccGyroMag BLE Characteristc */
  COPY_IP_W2ST_CHAR_UUID(uuid);
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret = aci_gatt_add_char(HWServW2STHandle, UUID_TYPE_128, &char_uuid, 64, CHAR_PROP_READ,
                          ATTR_PERMISSION_NONE, GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP, 16, 0,
                          &IpCharHandle);
  if (ret != BLE_STATUS_SUCCESS)
      return BLE_STATUS_ERROR;

  return BLE_STATUS_SUCCESS;
}

void ftoa(float n, char* res, int afterpoint) {
    // Extract integer part
    int j;
    int ipart = (int)n;

    // Extract floating part
    float fpart = n - (float)ipart;

    // Convert integer part to string
    sprintf(res, "%d", ipart);

    // Add decimal point if afterpoint is greater than 0
    if (afterpoint != 0) {
        int i = strlen(res);
        res[i] = '.';  // Add decimal point
        res[i + 1] = '\0';

        // Multiply fractional part to get it to integer
        for (j = 0; j < afterpoint; j++)
          fpart = fpart * 10;

        // Append fractional part after the decimal point
        sprintf(res + i + 1, "%d", (int)fpart);
    }
}

/**
 * @brief  Update environmental characteristic value
 * @param  th structure containing temprature and humidity.
 * @retval tBleStatus Status
 */
tBleStatus Environmental_Update(th_value_t th)
{
  tBleStatus ret;
  char buff[64],temp_str[10],humid_str[10];

  memset(buff, 0, 64);
  ftoa((float)th.temperature_milli_degC / 1000, temp_str, 1);
  ftoa((float)th.humidity_milli_RH / 1000, humid_str, 1);

  sprintf(buff, "T:%sC   H:%s%%  ", temp_str, humid_str);

  ret = aci_gatt_update_char_value(HWServW2STHandle, EnvironmentalCharHandle,
                                   0, strlen(buff), (uint8_t*)buff);

  if (ret != BLE_STATUS_SUCCESS){
    PRINT_DBG("Error while updating TEMP characteristic: 0x%04X\r\n",ret) ;
    return BLE_STATUS_ERROR ;
  }

  return BLE_STATUS_SUCCESS;
}

/**
 * @brief  Update acceleration characteristic value
 * @param  x_axes structure containing acceleration value in mg.
 * @retval tBleStatus Status
 */
tBleStatus Acc_Update(accelerometer_value_t x_axes)
{
  char buff[64];
  tBleStatus ret;
  memset(buff, 0, 64);
  sprintf(buff, "X%+03d   Y%+03d   Z%+03d   ", (int16_t)(x_axes.x / 170),
          (int16_t)(x_axes.y / 170), (int16_t)(x_axes.z / 170));

  ret = aci_gatt_update_char_value(HWServW2STHandle, AccCharHandle,
				   0, strlen(buff), (uint8_t*)buff);
  if (ret != BLE_STATUS_SUCCESS){
    PRINT_DBG("Error while updating Acceleration characteristic: 0x%02X\r\n",ret) ;
    return BLE_STATUS_ERROR ;
  }

  return BLE_STATUS_SUCCESS;
}


/**
 * @brief  Update IP characteristic value
 * @param  ip null terminated IP char string.
 * @param  gw null terminated Gateway IP char string.
 * @retval tBleStatus Status
 */
tBleStatus ble_IP_Update(const char *ip, const char *gw)
{
    tBleStatus ret;
    char buff[64];

    memset(buff, 0, sizeof(buff));
    sprintf(buff, "%s (gateway:%s)", ip, gw);

    ret = aci_gatt_update_char_value(HWServW2STHandle, IpCharHandle, 0, sizeof(buff),
                                     (uint8_t *)buff);

    if (ret != BLE_STATUS_SUCCESS)
    {
        PRINT_DBG("Error while updating IP characteristic: 0x%04X\r\n", ret);
        return BLE_STATUS_ERROR;
    }

    return BLE_STATUS_SUCCESS;
}

void update_ble_ip_gw(const char *ip, const char *gw)
{
    ble_IP_Update(ip, gw);
}

/**
 * @brief  Update the sensor value
 *
 * @param  Handle of the characteristic to update
 * @retval None
 */
void Read_Request_CB(uint16_t handle)
{
  tBleStatus ret;

  if(handle == AccCharHandle + 1)
  {
    Acc_Update(get_accelerometer_values());
  }
  else if (handle == EnvironmentalCharHandle + 1)
  {
    Environmental_Update(get_th_values());
  }

  if(connection_handle !=0)
  {
    ret = aci_gatt_allow_read(connection_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
      PRINT_DBG("aci_gatt_allow_read() failed: 0x%02x\r\n", ret);
    }
  }
}

/**
 * @brief  This function is called when there is a change on the gatt attribute.
 *         With this function it's possible to understand if one application
 *         is subscribed to the one service or not.
 *
 * @param  uint16_t att_handle Handle of the attribute
 * @param  uint8_t  *att_data attribute data
 * @param  uint8_t  data_length length of the data
 * @retval None
 */
void Attribute_Modified_Request_CB(uint16_t Connection_Handle, uint16_t attr_handle, uint16_t Offset, uint8_t data_length, uint8_t *att_data)
{
  if(attr_handle == EnvironmentalCharHandle + 2) {
    if (att_data[0] == 1) {
      send_env = TRUE;
    } else if (att_data[0] == 0){
      send_env = FALSE;
    }
  }
  else if (attr_handle == AccCharHandle +2) {
    if (att_data[0] == 1) {
      send_mot = TRUE;
    } else if (att_data[0] == 0){
      send_mot = FALSE;
    }
  }
}
