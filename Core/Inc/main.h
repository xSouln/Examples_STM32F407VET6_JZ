/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

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
#define CRITICAL_ENTER()

#define CRITICAL_EXIT()
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LABEL_PRINTER_TASK_STACK_SIZE (0x800/4)
#define ipISR_TASK_STACK_SIZE_WORDS (0x400/4)
#define NETWORK_WATCHDOG_TASK_STACK_SIZE (0x200/4)
#define STARTUP_TASK_STACK_SIZE (0x400/4)
#define FLASH_MANAGER_TASK_STACK_SIZE (0x400/4)
#define SHELL_TASK_STACK_SIZE (0x800/4)
#define WATCHDOG_TASK_STACK_SIZE (0x200/4)
#define LED_TASK_STACK_SIZE (0x200/4)
#define TEST_TASK_STACK_SIZE (0x400/4)
#define SNTP_TASK_STACK_SIZE (0x300/4)
#define MQTT_TASK_STACK_SIZE (0x1800/4)
#define HTTP_POST_TASK_STACK_SIZE (0x800/4)
#define HFU_TASK_STACK_SIZE (0x400/4)
#define HERMES_APPLICATION_TASK_STACK_SIZE (0x800/4)
#define rfISR_TASK_STACK_SIZE (0x600/4)
#define SURENET_TASK_STACK_SIZE (0x800/4)
#define WS18B20_Pin GPIO_PIN_2
#define WS18B20_GPIO_Port GPIOE
#define W25Q128_CS_Pin GPIO_PIN_3
#define W25Q128_CS_GPIO_Port GPIOE
#define SPI2_MISO_Pin GPIO_PIN_2
#define SPI2_MISO_GPIO_Port GPIOC
#define SPI2_MOSI_Pin GPIO_PIN_3
#define SPI2_MOSI_GPIO_Port GPIOC
#define NRF24L01_CE_Pin GPIO_PIN_7
#define NRF24L01_CE_GPIO_Port GPIOE
#define NRF24L01_CSN_Pin GPIO_PIN_8
#define NRF24L01_CSN_GPIO_Port GPIOE
#define NRF24L01_IRQ_Pin GPIO_PIN_9
#define NRF24L01_IRQ_GPIO_Port GPIOE
#define NRF24L01_IRQ_EXTI_IRQn EXTI9_5_IRQn
#define SW1_Pin GPIO_PIN_10
#define SW1_GPIO_Port GPIOE
#define SW2_Pin GPIO_PIN_11
#define SW2_GPIO_Port GPIOE
#define SW3_Pin GPIO_PIN_12
#define SW3_GPIO_Port GPIOE
#define LED1_Pin GPIO_PIN_13
#define LED1_GPIO_Port GPIOE
#define LED2_Pin GPIO_PIN_14
#define LED2_GPIO_Port GPIOE
#define LED3_Pin GPIO_PIN_15
#define LED3_GPIO_Port GPIOE
#define SPI2_SCK_Pin GPIO_PIN_10
#define SPI2_SCK_GPIO_Port GPIOB
#define USB_HS_DM_Pin GPIO_PIN_14
#define USB_HS_DM_GPIO_Port GPIOB
#define USB_HS_DP_Pin GPIO_PIN_15
#define USB_HS_DP_GPIO_Port GPIOB
#define AT86RF233_IRQ_Pin GPIO_PIN_12
#define AT86RF233_IRQ_GPIO_Port GPIOD
#define AT86RF233_IRQ_EXTI_IRQn EXTI15_10_IRQn
#define AT86RF233_RST_Pin GPIO_PIN_13
#define AT86RF233_RST_GPIO_Port GPIOD
#define AT86RF233_SLP_TR_Pin GPIO_PIN_14
#define AT86RF233_SLP_TR_GPIO_Port GPIOD
#define AT86RF233_CS_Pin GPIO_PIN_15
#define AT86RF233_CS_GPIO_Port GPIOD
#define SDIO_D0_Pin GPIO_PIN_8
#define SDIO_D0_GPIO_Port GPIOC
#define SDIO_D1_Pin GPIO_PIN_9
#define SDIO_D1_GPIO_Port GPIOC
#define USART1_TX_Pin GPIO_PIN_9
#define USART1_TX_GPIO_Port GPIOA
#define USART1_RX_Pin GPIO_PIN_10
#define USART1_RX_GPIO_Port GPIOA
#define USB_FS_DM_Pin GPIO_PIN_11
#define USB_FS_DM_GPIO_Port GPIOA
#define USB_FS_DP_Pin GPIO_PIN_12
#define USB_FS_DP_GPIO_Port GPIOA
#define SDIO_D2_Pin GPIO_PIN_10
#define SDIO_D2_GPIO_Port GPIOC
#define SDIO_D3_Pin GPIO_PIN_11
#define SDIO_D3_GPIO_Port GPIOC
#define SDIO_CK_Pin GPIO_PIN_12
#define SDIO_CK_GPIO_Port GPIOC
#define CAN1_RX_Pin GPIO_PIN_0
#define CAN1_RX_GPIO_Port GPIOD
#define CAN1_TX_Pin GPIO_PIN_1
#define CAN1_TX_GPIO_Port GPIOD
#define SDIO_CMD_Pin GPIO_PIN_2
#define SDIO_CMD_GPIO_Port GPIOD
#define SDCARD_CD_Pin GPIO_PIN_3
#define SDCARD_CD_GPIO_Port GPIOD
#define USART2_TX_Pin GPIO_PIN_5
#define USART2_TX_GPIO_Port GPIOD
#define USART2_RX_Pin GPIO_PIN_6
#define USART2_RX_GPIO_Port GPIOD
#define UASRT2_EN_Pin GPIO_PIN_7
#define UASRT2_EN_GPIO_Port GPIOD
#define CAN2_RX_Pin GPIO_PIN_5
#define CAN2_RX_GPIO_Port GPIOB
#define CAN2_TX_Pin GPIO_PIN_6
#define CAN2_TX_GPIO_Port GPIOB
#define I2C1_SCL_Pin GPIO_PIN_8
#define I2C1_SCL_GPIO_Port GPIOB
#define I2C1_SDA_Pin GPIO_PIN_9
#define I2C1_SDA_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
