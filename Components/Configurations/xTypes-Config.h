//==============================================================================
#ifndef _X_TYPES_CONFIG_H_
#define _X_TYPES_CONFIG_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "stm32f4xx_hal.h"
#include "Registers/registers.h"
//==============================================================================
//defines:

#define xUSART_USER_CONFIG 1
//==============================================================================
//types:

REG_GPIO_PORT_TYPEDEF(A,
		Pin0,
		ETH_REF_CLK,
		ETH_MDIO,
		Pin3,
		Pin4,
		Pin5,
		Pin6,
		ETH_CRS_DV,
		Pin8,
		USART1_TX,
		USART1_RX,
		USB_FS_DM,
		USB_FS_DP,
		SWDIO,
		SWDCLK,
		Pin15);

REG_GPIO_PORT_TYPEDEF(E,
		Pin0,
		Pin1,
		WS18B20,
		W25Q128_CS,
		Pin4,
		Pin5,
		Pin6,
		NRF24L01_CE,
		NRF24L01_CSN,
		NRF24L01_IRQ,
		SW1,
		SW2,
		SW3,
		LED1,
		LED2,
		LED3);

#define PortA ((REG_GPIO_PORT_A_T*)GPIOA)
#define PortE ((REG_GPIO_PORT_E_T*)GPIOE)
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_X_TYPES_CONFIG_H_
