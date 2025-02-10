/**
  ******************************************************************************
  * @file    app_bluenrg_2.c
  * @author  SRA Application Team
  * @brief   BlueNRG-2 initialization and applicative code
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
#include "app_bluenrg_2.h"

#include <stdlib.h>

#include "ble_sensor.h"
#include "bluenrg1_aci.h"
#include "bluenrg1_hci_le.h"
#include "bluenrg1_events.h"
#include "hci_tl.h"
#include "gatt_db.h"
#include "bluenrg_utils.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/**
 * This define enables a security scenario with PassKey Entry method.
 *
 * It is the BLE Central connected to this BLE Peripheral
 * that starts security process by sending a Slave Security Request.
 * Once connected with the BLE Central device, this BLE Peripheral starts
 * ACI_GAP_SLAVE_SECURITY_REQ procedure. The BLE Peripheral sets the key through
 * ACI_GAP_PASS_KEY_RESP command after the ACI_GAP_PASS_KEY_REQ_EVENT event is
 * generated.
 * At this stage, an input window is displayed on the Central device for allowing
 * user to insert the key set by the Peripheral device (123456 in this example).
 * After these operations the Pairing is completed and the bonded device is
 * displayed.
 */
#define SECURE_PAIRING (0)
#define PERIPHERAL_PASS_KEY (123456)

/**
 * 1 to send environmental and motion data when pushing the user button
 * 0 to send environmental and motion data automatically (period = 1 sec)
 */
#define USE_BUTTON (0)

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

extern __IO uint16_t connection_handle;
extern volatile uint8_t set_connectable;
extern volatile uint8_t connected;
extern volatile uint8_t pairing;
extern volatile uint8_t paired;
uint8_t bdaddr[BDADDR_SIZE];
extern __IO uint8_t send_env;
extern __IO uint8_t send_mot;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void User_Process(void);
static uint8_t Sensor_DeviceInit(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */


void MX_BlueNRG_2_Init(void)
{
  /* USER CODE BEGIN SV */

  /* USER CODE END SV */

  /* USER CODE BEGIN BlueNRG_2_Init_PreTreatment */

  /* USER CODE END BlueNRG_2_Init_PreTreatment */

  /* Initialize the peripherals and the BLE Stack */
  uint8_t ret;
  hci_init(APP_UserEvtRx, NULL);

  PRINT_DBG("BlueNRG-2 SensorDemo_BLESensor-App Application\r\n");

  /* Init Sensor Device */
  ret = Sensor_DeviceInit();
  if (ret != BLE_STATUS_SUCCESS)
  {
    PRINT_DBG("SensorDeviceInit()--> Failed 0x%02x\r\n", ret);
  }

  PRINT_DBG("BLE Stack Initialized & Device Configured\r\n");

}

/*
 * BlueNRG-2 background task
 */
void MX_BlueNRG_2_Process(void)
{

  hci_user_evt_proc();
  User_Process();
}


/**
 * @brief  Initialize the device sensors
 *
 * @param  None
 * @retval None
 */
uint8_t Sensor_DeviceInit(void)
{
  uint8_t ret;
  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  uint8_t device_name[] = {SENSOR_DEMO_NAME};
  uint8_t  hwVersion;
  uint16_t fwVersion;
  uint8_t  bdaddr_len_out;
  uint8_t  config_data_stored_static_random_address = 0x80; /* Offset of the static random address stored in NVM */

  /* Sw reset of the device */
  hci_reset();
  /**
   *  To support both the BlueNRG-2 and the BlueNRG-2N a minimum delay of 2000ms is required at device boot
   */
  HAL_Delay(2000);

  /* get the BlueNRG HW and FW versions */
  getBlueNRGVersion(&hwVersion, &fwVersion);

  PRINT_DBG("HWver %d\nFWver %d\r\n", hwVersion, fwVersion);

  ret = aci_hal_read_config_data(config_data_stored_static_random_address,
                                 &bdaddr_len_out, bdaddr);

  if (ret) {
    PRINT_DBG("Read Static Random address failed.\r\n");
  }

  if ((bdaddr[5] & 0xC0) != 0xC0) {
    PRINT_DBG("Static Random address not well formed.\r\n");
  }

  ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,
                                  bdaddr_len_out,
                                  bdaddr);
  if (ret != BLE_STATUS_SUCCESS) {
    PRINT_DBG("aci_hal_write_config_data() failed:0x%02x\r\n", ret);
  }
  else {
    PRINT_DBG("aci_hal_write_config_data --> SUCCESS\r\n");
  }

  /* Set the TX power -2 dBm */
  aci_hal_set_tx_power_level(1, 4);
  if (ret != BLE_STATUS_SUCCESS) {
    PRINT_DBG("aci_hal_set_tx_power_level() failed:0x%02x\r\n", ret);
  }
  else {
    PRINT_DBG("aci_hal_set_tx_power_level --> SUCCESS\r\n");
  }

  /* GATT Init */
  ret = aci_gatt_init();
  if (ret != BLE_STATUS_SUCCESS) {
    PRINT_DBG("aci_gatt_init() failed: 0x%02x\r\n", ret);
    return ret;
  }
  else {
    PRINT_DBG("aci_gatt_init() --> SUCCESS\r\n");
  }

  /* GAP Init */
  ret = aci_gap_init(GAP_PERIPHERAL_ROLE, 0x00, sizeof(device_name), &service_handle, &dev_name_char_handle,
                     &appearance_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    PRINT_DBG("aci_gap_init() failed: 0x%02x\r\n", ret);
    return ret;
  }
  else {
    PRINT_DBG("aci_gap_init() --> SUCCESS\r\n");
  }

  /* Update device name */
  ret = aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0, sizeof(device_name),
                                   device_name);
  if (ret != BLE_STATUS_SUCCESS) {
    PRINT_DBG("aci_gatt_update_char_value() failed: 0x%02x\r\n", ret);
    return ret;
  }
  else {
    PRINT_DBG("aci_gatt_update_char_value() --> SUCCESS\r\n");
  }

  /*
   * Clear security database: this implies that each time the application is executed
   * the full bonding process is executed (with PassKey generation and setting).
   */
  ret = aci_gap_clear_security_db();
  if (ret != BLE_STATUS_SUCCESS) {
    PRINT_DBG("aci_gap_clear_security_db() failed:0x%02x\r\n", ret);
  }
  else {
    PRINT_DBG("aci_gap_clear_security_db --> SUCCESS\r\n");
  }

  /*
   * Set the I/O capability otherwise the Central device (e.g. the smartphone) will
   * propose a PIN that will be accepted without any control.
   */
  if (aci_gap_set_io_capability(IO_CAP_DISPLAY_ONLY)==BLE_STATUS_SUCCESS) {
    PRINT_DBG("I/O Capability Configurated\r\n");
  } else {
    PRINT_DBG("Error Setting I/O Capability\r\n");
  }

  /* BLE Security v4.2 is supported: BLE stack FW version >= 2.x (new API prototype) */
  ret = aci_gap_set_authentication_requirement(BONDING,
                                               MITM_PROTECTION_REQUIRED,
                                               SC_IS_SUPPORTED,
                                               KEYPRESS_IS_NOT_SUPPORTED,
                                               7,
                                               16,
                                               DONOT_USE_FIXED_PIN_FOR_PAIRING,
                                               PERIPHERAL_PASS_KEY,
                                               0x00); /* - 0x00: Public Identity Address
                                                         - 0x01: Random (static) Identity Address */
  if (ret != BLE_STATUS_SUCCESS) {
    PRINT_DBG("aci_gap_set_authentication_requirement()failed: 0x%02x\r\n", ret);
    return ret;
  }
  else {
    PRINT_DBG("aci_gap_set_authentication_requirement() --> SUCCESS\r\n");
  }

  PRINT_DBG("BLE Stack Initialized with SUCCESS\r\n");

  ret = Add_HWServW2ST_Service();
  if (ret == BLE_STATUS_SUCCESS) {
    PRINT_DBG("BlueNRG2 HW service added successfully.\r\n");
  }
  else {
    PRINT_DBG("Error while adding BlueNRG2 HW service: 0x%02x\r\n", ret);
    while(1);
  }
  return BLE_STATUS_SUCCESS;
}

/**
 * @brief  User Process
 *
 * @param  None
 * @retval None
 */
static void User_Process(void)
{
  uint8_t ret;
  static uint8_t counter=0;

  /* Make the device discoverable */
  if(set_connectable)
  {
    Set_DeviceConnectable();
    set_connectable = FALSE;
  }

  if ((connected) && (!pairing))
  {
    ret = aci_gap_slave_security_req(connection_handle);
    if (ret != BLE_STATUS_SUCCESS) {
      PRINT_DBG("aci_gap_slave_security_req() failed:0x%02x\r\n", ret);
    }
    else {
      PRINT_DBG("aci_gap_slave_security_req --> SUCCESS\r\n");
    }
    pairing = TRUE;
  }

  /*  Update sensor value */

    if (paired)
    {
      /* Set a random seed */
      srand(HAL_GetTick());

      if (send_env && !counter) {
        /* Update Environmental data */
        Environmental_Update(get_th_values());
      }

      if (send_mot && !counter) {
        /* Update emulated Acceleration, Gyroscope and Sensor Fusion data */
        Acc_Update(get_accelerometer_values());
      }
    }
    if (counter++ > 20)
      counter = 0;
}

/**
 * @brief  Get hardware and firmware version
 *
 * @param  Hardware version
 * @param  Firmware version
 * @retval Status
 */
uint8_t getBlueNRGVersion(uint8_t *hwVersion, uint16_t *fwVersion)
{
  uint8_t status;
  uint8_t hci_version, lmp_pal_version;
  uint16_t hci_revision, manufacturer_name, lmp_pal_subversion;

  status = hci_read_local_version_information(&hci_version, &hci_revision, &lmp_pal_version,
                                              &manufacturer_name, &lmp_pal_subversion);

  if (status == BLE_STATUS_SUCCESS) {
    *hwVersion = hci_revision >> 8;
    *fwVersion = (hci_revision & 0xFF) << 8;              // Major Version Number
    *fwVersion |= ((lmp_pal_subversion >> 4) & 0xF) << 4; // Minor Version Number
    *fwVersion |= lmp_pal_subversion & 0xF;               // Patch Version Number
  }
  return status;
}


/* ***************** BlueNRG-1 Stack Callbacks ********************************/
/**
 * @brief  This event indicates that a new connection has been created
 *
 * @param  See file bluenrg1_events.h
 * @retval See file bluenrg1_events.h
 */
void hci_le_connection_complete_event(uint8_t Status,
                                      uint16_t Connection_Handle,
                                      uint8_t Role,
                                      uint8_t Peer_Address_Type,
                                      uint8_t Peer_Address[6],
                                      uint16_t Conn_Interval,
                                      uint16_t Conn_Latency,
                                      uint16_t Supervision_Timeout,
                                      uint8_t Master_Clock_Accuracy)
{
  connected = TRUE;
#if (!SECURE_PAIRING)
  pairing = TRUE;
  paired = TRUE;
#endif
  connection_handle = Connection_Handle;

  PRINT_DBG("Connected (%02x %02x %02x %02x %02x %02x)\r\n", Peer_Address[5], Peer_Address[4], Peer_Address[3],
                                                             Peer_Address[2], Peer_Address[1], Peer_Address[0]);

}

/**
 * @brief  This event occurs when a connection is terminated
 *
 * @param  See file bluenrg1_events.h
 * @retval See file bluenrg1_events.h
 */
void hci_disconnection_complete_event(uint8_t Status,
                                      uint16_t Connection_Handle,
                                      uint8_t Reason)
{
  connected = FALSE;
  pairing = FALSE;
  paired = FALSE;

  /* Make the device connectable again */
  set_connectable = TRUE;
  connection_handle = 0;
  PRINT_DBG("Disconnected (0x%02x)\r\n", Reason);
}

/**
 * @brief  This event is given when a read request is received
 *         by the server from the client
 * @param  See file bluenrg1_events.h
 * @retval See file bluenrg1_events.h
 */
void aci_gatt_read_permit_req_event(uint16_t Connection_Handle,
                                    uint16_t Attribute_Handle,
                                    uint16_t Offset)
{
  Read_Request_CB(Attribute_Handle);
}

/**
 * @brief  This event is given when an attribute changes his value
 * @param  See file bluenrg1_events.h
 * @retval See file bluenrg1_events.h
 */
void aci_gatt_attribute_modified_event(uint16_t Connection_Handle,
                                       uint16_t Attribute_Handle,
                                       uint16_t Offset,
                                       uint16_t Attr_Data_Length,
                                       uint8_t Attr_Data[])
{
  Attribute_Modified_Request_CB(Connection_Handle, Attribute_Handle, Offset, Attr_Data_Length, Attr_Data);
}

/**
 * @brief  This event is generated by the Security manager to the application
 *         when a passkey is required for pairing.
 *         When this event is received, the application has to respond with the
 *         aci_gap_pass_key_resp command.
 * @param  See file bluenrg1_events.h
 * @retval See file bluenrg1_events.h
 */
void aci_gap_pass_key_req_event(uint16_t Connection_Handle)
{
  uint8_t ret;

  ret = aci_gap_pass_key_resp(connection_handle, PERIPHERAL_PASS_KEY);
  if (ret != BLE_STATUS_SUCCESS) {
    PRINT_DBG("aci_gap_pass_key_resp failed:0x%02x\r\n", ret);
  } else {
    PRINT_DBG("aci_gap_pass_key_resp OK\r\n");
  }
}

/**
 * @brief  This event is generated when the pairing process has completed successfully or a pairing
 *         procedure timeout has occurred or the pairing has failed. This is to notify the application that
 *         we have paired with a remote device so that it can take further actions or to notify that a
 *         timeout has occurred so that the upper layer can decide to disconnect the link.
 * @param  See file bluenrg1_events.h
 * @retval See file bluenrg1_events.h
 */
void aci_gap_pairing_complete_event(uint16_t connection_handle, uint8_t status, uint8_t reason)
{
  if (status == 0x02) { /* Pairing Failed */
    PRINT_DBG("aci_gap_pairing_complete_event failed:0x%02x with reason 0x%02x\r\n", status, reason);
  }
  else {
    paired = TRUE;
    PRINT_DBG("aci_gap_pairing_complete_event with status 0x%02x\r\n", status);
  }
}
