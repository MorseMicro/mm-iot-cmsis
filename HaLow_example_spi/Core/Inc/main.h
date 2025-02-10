/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u5xx_hal.h"
#include "stm32u5xx_ll_dma.h"
#include "stm32u5xx_ll_lptim.h"
#include "stm32u5xx_ll_lpuart.h"
#include "stm32u5xx_ll_rcc.h"
#include "stm32u5xx_ll_rtc.h"
#include "stm32u5xx_ll_spi.h"
#include "stm32u5xx_ll_system.h"
#include "stm32u5xx_ll_gpio.h"
#include "stm32u5xx_ll_exti.h"
#include "stm32u5xx_ll_lpgpio.h"
#include "stm32u5xx_ll_bus.h"
#include "stm32u5xx_ll_cortex.h"
#include "stm32u5xx_ll_utils.h"
#include "stm32u5xx_ll_pwr.h"

#include "hci_tl_interface.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define MMHAL_MAC_ADDR_OVERRIDE_ENABLED
#define JPEG_BUFFER_TIMEOUT_MS  (500)

#define SPI_PERIPH (SPI2)
#define SPI_DMA_PERIPH (GPDMA1)
#define SPI_RX_DMA_CHANNEL  (LL_DMA_CHANNEL_14)
#define SPI_TX_DMA_CHANNEL  (LL_DMA_CHANNEL_15)

#define SPI_IRQn            (EXTI15_IRQn)
#define SPI_IRQ_LINE        (LL_EXTI_LINE_15)
#define SPI_IRQ_HANDLER     EXTI15_IRQHandler
#define BUSY_IRQn           (EXTI5_IRQn)
#define BUSY_IRQ_LINE       (LL_EXTI_LINE_5)
#define BUSY_IRQ_HANDLER    EXTI5_IRQHandler

#define LOG_USART               (LPUART1)
#define LOG_USART_IRQ           (LPUART1_IRQn)
#define LOG_USART_IRQ_HANDLER   LPUART1_IRQHandler

#define BCF_VFEM_THRESHOLD  9500
#define BCF_DATA_3V3        bcf_aw_hm593
#define BCF_DATA_3V3_LEN    bcf_aw_hm593_len
#define BCF_DATA_4V3        bcf_aw_hm593_4v3
#define BCF_DATA_4V3_LEN    bcf_aw_hm593_4v3_len
/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
void MX_TIM4_Init(void);
void MX_I2C1_Init(void);
void MX_DCMI_Init(void);
void MX_OCTOSPI1_Init(void);

/* USER CODE BEGIN EFP */
extern DCMI_HandleTypeDef hdcmi;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CAM_RESET_Pin LL_GPIO_PIN_2
#define CAM_RESET_GPIO_Port GPIOE
#define CAM_PWDN_Pin LL_GPIO_PIN_3
#define CAM_PWDN_GPIO_Port GPIOE
#define SPARE_GPIO_PC13_Pin LL_GPIO_PIN_13
#define SPARE_GPIO_PC13_GPIO_Port GPIOC
#define LOG_USART_RX_Pin LL_GPIO_PIN_0
#define LOG_USART_RX_GPIO_Port GPIOC
#define LOG_USART_TX_Pin LL_GPIO_PIN_1
#define LOG_USART_TX_GPIO_Port GPIOC
#define ADC_VFEM_Pin LL_GPIO_PIN_2
#define ADC_VFEM_GPIO_Port GPIOC
#define USER_BUTTON_Pin LL_GPIO_PIN_3
#define USER_BUTTON_GPIO_Port GPIOC
#define BRNG_SPI_IRQ_Pin LL_GPIO_PIN_1
#define BRNG_SPI_IRQ_GPIO_Port GPIOA
#define BRNG_SPI_IRQ_EXTI_IRQn EXTI1_IRQn
#define GPIO_LED_GREEN_Pin LL_GPIO_PIN_7
#define GPIO_LED_GREEN_GPIO_Port GPIOE
#define GPIO_LED_BLUE_Pin LL_GPIO_PIN_8
#define GPIO_LED_BLUE_GPIO_Port GPIOE
#define GPIO_LED_RED_Pin LL_GPIO_PIN_11
#define GPIO_LED_RED_GPIO_Port GPIOE
#define SPI_IRQ_Pin LL_GPIO_PIN_15
#define SPI_IRQ_GPIO_Port GPIOB
#define SPI_IRQ_EXTI_IRQn EXTI15_IRQn
#define SPARE_GPIO_PD11_Pin LL_GPIO_PIN_11
#define SPARE_GPIO_PD11_GPIO_Port GPIOD
#define RGB_LED_R_Pin LL_GPIO_PIN_12
#define RGB_LED_R_GPIO_Port GPIOD
#define RGB_LED_G_Pin LL_GPIO_PIN_13
#define RGB_LED_G_GPIO_Port GPIOD
#define RGB_LED_B_Pin LL_GPIO_PIN_14
#define RGB_LED_B_GPIO_Port GPIOD
#define SPARE_GPIO_PD15_Pin LL_GPIO_PIN_15
#define SPARE_GPIO_PD15_GPIO_Port GPIOD
#define CAMERA_XCLK_Pin LL_GPIO_PIN_8
#define CAMERA_XCLK_GPIO_Port GPIOA
#define MM_DEBUG_1_Pin LL_GPIO_PIN_9
#define MM_DEBUG_1_GPIO_Port GPIOA
#define MM_DEBUG_0_Pin LL_GPIO_PIN_10
#define MM_DEBUG_0_GPIO_Port GPIOA
#define SPARE_GPIO_PA11_Pin LL_GPIO_PIN_11
#define SPARE_GPIO_PA11_GPIO_Port GPIOA
#define BRNG_CS_Pin LL_GPIO_PIN_15
#define BRNG_CS_GPIO_Port GPIOA
#define WAKE_Pin LL_GPIO_PIN_0
#define WAKE_GPIO_Port GPIOD
#define SPI_SCK_Pin LL_GPIO_PIN_1
#define SPI_SCK_GPIO_Port GPIOD
#define BRNG_NRST_Pin LL_GPIO_PIN_2
#define BRNG_NRST_GPIO_Port GPIOD
#define SPI_MISO_Pin LL_GPIO_PIN_3
#define SPI_MISO_GPIO_Port GPIOD
#define SPI_MOSI_Pin LL_GPIO_PIN_4
#define SPI_MOSI_GPIO_Port GPIOD
#define VFEM_SHUNT_TOGGLE_Pin LL_GPIO_PIN_5
#define VFEM_SHUNT_TOGGLE_GPIO_Port GPIOD
#define SPI_CS_Pin LL_GPIO_PIN_4
#define SPI_CS_GPIO_Port GPIOB
#define BUSY_Pin LL_GPIO_PIN_5
#define BUSY_GPIO_Port GPIOB
#define BUSY_EXTI_IRQn EXTI5_IRQn
#define RESET_N_Pin LL_GPIO_PIN_0
#define RESET_N_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */
#define JPEG_BUFFER_SIZE  (32*1024)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
