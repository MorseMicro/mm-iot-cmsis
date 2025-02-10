/*
 * Copyright 2024 Morse Micro
 *
 * This file is licensed under terms that can be found in the LICENSE.md file in
 * the root directory of the Morse Micro IoT SDK software package.
 */
#include "demo_accelerometer.h"
#include "demo_temperature.h"
#include "mmutils.h"
#include "mmosal.h"
#include "mmhal.h"
#include <string.h>
#include "main.h"
#include "app_bluenrg_2.h"
#include "ekh05_camera.h"
#include "gatt_db.h"
#include "w25q16jv.h"

static struct mmosal_semb *JPEG_buffer_lock = NULL;
static struct mmosal_semb *QSPI_flash_lock = NULL;
struct mmosal_task *periphs_task_p;
static uint8_t got_sof = 0; /*got Start Of Frame*/
uint32_t image_data_size = 0;
uint8_t *image_data_ptr = NULL;
uint8_t JPEG_buffer[JPEG_BUFFER_SIZE];

extern TIM_HandleTypeDef htim4;
extern OSPI_HandleTypeDef hospi1;

#define SAVED_IMAGE_DATA_SIZE *(uint32_t*)OCTOSPI1_BASE
#define ERASE_IMAGE_BOTTUN_WAIT_SECONDS(x) (x * 2)

enum mmhal_deep_sleep_veto_id
{
    MMHAL_VETO_ID_APP_CAMERA = MMHAL_VETO_ID_APP_MIN,
};

const uint8_t *periphs_get_jpeg_buffer(void) { return JPEG_buffer; }
uint32_t periphs_get_live_jpeg_size(void) { return image_data_size; }

bool periphs_qspi_is_memmap_running (void){
    return HAL_OSPI_GetState(&hospi1) == HAL_OSPI_STATE_BUSY_MEM_MAPPED;
}

bool periphs_jpeg_buffer_lock(void)
{
    if (JPEG_buffer_lock != NULL)
    {
        return mmosal_semb_wait(JPEG_buffer_lock, JPEG_BUFFER_TIMEOUT_MS);
    }
    return false;
}

void periphs_jpeg_buffer_unlock(void)
{
    if (JPEG_buffer_lock != NULL)
    {
        mmosal_semb_give(JPEG_buffer_lock);
    }
}

bool periphs_qspi_flash_lock(void)
{
    if (QSPI_flash_lock != NULL)
    {
        return mmosal_semb_wait(QSPI_flash_lock, JPEG_BUFFER_TIMEOUT_MS);
    }
    return false;
}

void periphs_qspi_flash_unlock(void)
{
    if (QSPI_flash_lock != NULL)
    {
        mmosal_semb_give(QSPI_flash_lock);
    }
}

void http_file_sent(const void *end_address)
{
    /*if the end_address is the jpeg image end, then unlock the jpeg buffer.*/
    if ((uint8_t *)end_address == (JPEG_buffer + image_data_size))
    {
        periphs_jpeg_buffer_unlock();
        return;
    }
    /* don't check the address if the memory mapped isn't on-going.*/
    if (periphs_qspi_is_memmap_running())
    {
        if ((uint8_t *)end_address == ((uint8_t *)OCTOSPI1_BASE + sizeof(uint32_t) + SAVED_IMAGE_DATA_SIZE))
        {
            periphs_qspi_flash_unlock();
            return;
        }
    }
}

static void flash_image_cleanup(bool lock_jpeg_buff)
{
    if (QSPI_Init(&hospi1) != HAL_OK)
    {
        printf("QSPI init failed\n");
    }
    else
    {
        if (QSPI_EnableMemoryMappedMode(&hospi1) != HAL_OK)
        {
            printf("QSPI map failed\n");
        }
    }
    htim4.Instance->CCR1 = 0;
    htim4.Instance->CCR2 = 0;
    htim4.Instance->CCR3 = 0;
    if(lock_jpeg_buff)
    {
        periphs_jpeg_buffer_unlock();
    }
    periphs_qspi_flash_unlock();
}

static void erase_image(void)
{
    /*lock the image semaphores*/
    if (!periphs_qspi_flash_lock())
    {
        printf("periphs_qspi_flash_lock failed.\n");
        return;
    }
    /*Turn on the LED white*/
    htim4.Instance->CCR1 = 0;
    htim4.Instance->CCR2 = 0;
    htim4.Instance->CCR3 = 1000;

    /*disable the memory-mapped mode of the QSPI flash*/
    if(HAL_OSPI_Abort(&hospi1) != HAL_OK)
    {
        printf("unable to disable QSPI memory-mapping.\n");
        periphs_qspi_flash_unlock();
        return;
    }
    /*erase the part of flash that the image is going to sit*/
    if (QSPI_WriteEnable(&hospi1) != HAL_OK)
    {
        /*Since we have the same fail message multiple places, we're using the number in the bracket
         * to know which one has failed.*/
        printf("[0] QSPI_WriteEnable failed.\n");
        flash_image_cleanup(false);
        return;
    }

    if (QSPI_BlockSectorErase(&hospi1, 0) != HAL_OK)
    {
        printf("QSPI_EraseSector failed.\n");
        flash_image_cleanup(false);
        return;
    }
    mmosal_task_sleep(800);
    htim4.Instance->CCR1 = 0;
    htim4.Instance->CCR2 = 0;
    htim4.Instance->CCR3 = 0;
    flash_image_cleanup(false);
}

static void save_image(void)
{
    uint8_t *data_to_write;
    uint32_t remaining_bytes;
    /*check if there's an image and not too big or small*/
    if ((image_data_size < 256) || (image_data_size > 65532) || image_data_ptr == NULL)
    {
        printf("No image!\n");
        return;
    }
    /*lock the image semaphores*/
    if (!periphs_jpeg_buffer_lock())
    {
        printf("periphs_jpeg_buffer_lock failed.\n");
        return;
    }
    if (!periphs_qspi_flash_lock())
    {
        printf("periphs_qspi_flash_lock failed.\n");
        periphs_jpeg_buffer_unlock();
        return;
    }
    /*Turn on the LED white*/
    htim4.Instance->CCR1 = 1000;
    htim4.Instance->CCR2 = 1000;
    htim4.Instance->CCR3 = 1000;

    /*disable the memory-mapped mode of the QSPI flash*/
    if(HAL_OSPI_Abort(&hospi1) != HAL_OK)
    {
        printf("unable to disable QSPI memory-mapping.\n");
        periphs_jpeg_buffer_unlock();
        periphs_qspi_flash_unlock();
        return;
    }
    /*erase the part of flash that the image is going to sit*/
    if (QSPI_WriteEnable(&hospi1) != HAL_OK)
    {
        /*Since we have the same fail message multiple places, we're using the number in the bracket
         * to know which one has failed.*/
        printf("[0] QSPI_WriteEnable failed.\n");
        flash_image_cleanup(true);
        return;
    }

    if (QSPI_BlockSectorErase(&hospi1, 0) != HAL_OK)
    {
        printf("QSPI_EraseSector failed.\n");
        flash_image_cleanup(true);
        return;
    }
    /*Write the image size to the flash. We are going to save the image size in the very begining of
     * flash, then will add the image data after.*/
    if (QSPI_WriteEnable(&hospi1) != HAL_OK)
    {
        printf("[1] QSPI_WriteEnable failed.\n");
    }
    if (QSPI_ProgramPage(&hospi1, 0, (uint8_t *)&image_data_size, 4) != HAL_OK)
    {
        printf("[1] QSPI_ProgramPage failed.\n");
    }
    /*write the image buffer to the flash.*/
    printf("Write %ld bytes from 0x%lx to external flash:", image_data_size,
           (uint32_t)image_data_ptr);

    data_to_write   = image_data_ptr;
    remaining_bytes = (image_data_ptr + image_data_size) - data_to_write;

    /* Write the first 256-4 bytes to first flash page. (we have to write data page by page. since
     * we are using the first 4 bytes of the first page to store the size, we can cony write 256-4
     * bytes to the first page.)*/
    if (QSPI_WriteEnable(&hospi1) != HAL_OK)
    {
        printf("[2] QSPI_WriteEnable failed.\n");
    }
    if (QSPI_ProgramPage(&hospi1, sizeof(uint32_t), data_to_write, 256 - 4) != HAL_OK)
    {
        printf("[2] QSPI_ProgramPage failed.\n");
    }
    data_to_write += (256 - 4);
    remaining_bytes = (image_data_ptr + image_data_size) - data_to_write;

    while (remaining_bytes)
    {
        if (QSPI_WriteEnable(&hospi1) != HAL_OK)
        {
            printf("[3] QSPI_WriteEnable failed.\n");
        }
        if ((remaining_bytes) >= 256)
        {
            if (QSPI_ProgramPage(&hospi1,
                                 sizeof(uint32_t) + (uint32_t)(data_to_write - image_data_ptr),
                                 data_to_write, 256) != HAL_OK)
            {
                printf("[3] QSPI_ProgramPage failed.\n");
                flash_image_cleanup(true);
                return;
            }
            data_to_write += 256;
            remaining_bytes -= 256;
        }
        else
        {
            if (QSPI_ProgramPage(&hospi1,
                                 sizeof(uint32_t) + (uint32_t)(data_to_write - image_data_ptr),
                                 data_to_write, remaining_bytes) != HAL_OK)
            {
                printf("[4] QSPI_ProgramPage failed.\n");
                flash_image_cleanup(true);
                return;
            }
            break;
        }
        printf(".");
    }
    printf("Finished successfully.\n");

    /*enable memore-mapped mode*/
    /*unlock the image semaphore.*/
    flash_image_cleanup(true);

}

void BSP_CAMERA_VsyncEventCallback(uint32_t Instance)
{
    if (got_sof)
    {
        BSP_CAMERA_Stop(Instance);
        image_data_size = hdcmi.DMA_Handle->Instance->CDAR - (uint32_t)JPEG_buffer;
        image_data_ptr = JPEG_buffer;
        got_sof = 0;
        mmosal_semb_give_from_isr(JPEG_buffer_lock);
    }
    else
    {
        got_sof = 1;
    }
}

static void periphs_task(void *arg)
{
    MM_UNUSED(arg);
    int counter = 0;
    while (1)
    {
        accelerometer_process();
        MX_BlueNRG_2_Process();

        if (counter++ == 40)
        {
            counter = 0;
            if (mmosal_semb_wait(JPEG_buffer_lock, JPEG_BUFFER_TIMEOUT_MS))
            {
                BSP_CAMERA_Start(0, JPEG_buffer, DCMI_MODE_SNAPSHOT);
            }
            temperature_process();
        }

        if (!HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin))
        {
            counter = 0;
            while (!HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) && counter < ERASE_IMAGE_BOTTUN_WAIT_SECONDS(4))
            {
                htim4.Instance->CCR1 = 0;
                htim4.Instance->CCR2 = 0;
                htim4.Instance->CCR3 = 0;
                mmosal_task_sleep(480);
                htim4.Instance->CCR1 = 1000;
                htim4.Instance->CCR2 = 1000;
                htim4.Instance->CCR3 = 1000;
                mmosal_task_sleep(20);
                counter++;
            }
            if (counter < ERASE_IMAGE_BOTTUN_WAIT_SECONDS(4))
            {
                save_image();
            }
            else
            {
                erase_image();
            }
            counter = 0;
            /*Stop Here until the user releases the button.*/
            while (!HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin));
        }
        mmosal_task_sleep(52);
    }
}

void periphs_start(void)
{
    /* We don't want the host to sleep. If it does the RGB will flicker*/
    mmhal_set_deep_sleep_veto(MMHAL_VETO_ID_APP_CAMERA);
    MMOSAL_ASSERT(JPEG_buffer_lock == NULL);
    JPEG_buffer_lock = mmosal_semb_create("JPEG_buffer_lock");
    MMOSAL_ASSERT(QSPI_flash_lock == NULL);
    QSPI_flash_lock = mmosal_semb_create("QSPI_flash_lock");
    /*give a free semaphore, so it can be taken.*/
    periphs_qspi_flash_unlock();
    /*Initialize the host peripherals for Demo example.*/
    MX_TIM4_Init();
    MX_I2C1_Init();
    MX_DCMI_Init();
    MX_OCTOSPI1_Init();

    if (QSPI_Init(&hospi1) != HAL_OK)
    {
        printf("QSPI init failed\n");
    }
    else
    {
        if (QSPI_EnableMemoryMappedMode(&hospi1) != HAL_OK)
        {
            printf("QSPI map failed\n");
        }
    }
    accelerometer_init();
    temperature_init();
    MX_BlueNRG_2_Init();
    /*At the start set BLE reported IP and GW to NOT-CONNECTED*/
    update_ble_ip_gw("NOT-CONNECTED", "NOT-CONNECTED");
    if (BSP_CAMERA_Init(0, CAMERA_R320x240, CAMERA_PF_JPEG, JPEG_BUFFER_SIZE) != BSP_ERROR_NONE)
    {
        printf("\n\nFailed to init camera.\n\n");
    }
    BSP_CAMERA_SetLightMode(0, CAMERA_LIGHT_HOME);
    BSP_CAMERA_Start(0, JPEG_buffer, DCMI_MODE_SNAPSHOT);
    printf("\n\nStarting Sensor Task\n\n");
    /* Read samples in polling mode (no int) */

    periphs_task_p = mmosal_task_create(periphs_task, NULL, MMOSAL_TASK_PRI_LOW, 1024, "sensors");
    if (periphs_task_p == NULL)
    {
        printf("Unable to start sensor task\n\r");
    }
}

uint8_t periphs_toggle_leds()
{
    static uint8_t counter = 0;
    if (++counter > 2)
        counter = 0;
    switch (counter)
    {
    case 0:
        HAL_GPIO_WritePin(GPIO_LED_BLUE_GPIO_Port, GPIO_LED_BLUE_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIO_LED_GREEN_GPIO_Port, GPIO_LED_GREEN_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIO_LED_RED_GPIO_Port, GPIO_LED_RED_Pin, GPIO_PIN_SET);
        break;
    case 1:
        HAL_GPIO_WritePin(GPIO_LED_BLUE_GPIO_Port, GPIO_LED_BLUE_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIO_LED_GREEN_GPIO_Port, GPIO_LED_GREEN_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIO_LED_RED_GPIO_Port, GPIO_LED_RED_Pin, GPIO_PIN_RESET);
        break;
    case 2:
        HAL_GPIO_WritePin(GPIO_LED_BLUE_GPIO_Port, GPIO_LED_BLUE_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIO_LED_GREEN_GPIO_Port, GPIO_LED_GREEN_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIO_LED_RED_GPIO_Port, GPIO_LED_RED_Pin, GPIO_PIN_RESET);
        break;

    default:
        break;
    }
    return counter;
}
