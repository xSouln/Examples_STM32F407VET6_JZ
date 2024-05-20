//==============================================================================
//header:

#ifndef _X_GPIO_CONFIG_H_
#define _X_GPIO_CONFIG_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Registers/registers.h"
//==============================================================================
//defines:

enum
{
	xGPIO_MAX_NUMBER_OF_PORTS
};
//------------------------------------------------------------------------------

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
//------------------------------------------------------------------------------
REG_GPIO_PORT_TYPEDEF(B,
		Pin0,
		Pin1,
		Pin2,
		Pin3,
		Pin4,
		CAN2_RX,
		CAN2_TX,
		Pin7,
		I2C_SCL,
		I2C_SDA,
		SPI2_SCK,
		ETH_TX_EN,
		ETH_TXD0,
		ETH_TXD1,
		USB_HS_DM,
		USB_HS_DP);
//------------------------------------------------------------------------------
REG_GPIO_PORT_TYPEDEF(C,
		Pin0,
		ETH_MDC,
		SPI2_MISO,
		SPI2_MOSI,
		ETH_RXD0,
		ETH_RXD1,
		USART6_TX,
		USART6_RX,
		SDIO_D0,
		SDIO_D1,
		SDIO_D2,
		SDIO_D3,
		SDIO_CK,
		Pin13,
		RCC_OSC32_IN,
		RCC_OSC32_OUT);
//------------------------------------------------------------------------------
REG_GPIO_PORT_TYPEDEF(D,
		CAN1_RX,
		CAN1_TX,
		SDIO_CMD,
		SDCARD_CD,
		Pin4,
		USART2_TX,
		USART2_RX,
		USART2_EN,
		USART3_TX,
		USART3_RX,
		Pin10,
		Pin11,
		Pin12,
		Pin13,
		Pin14,
		Pin15);
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
#define PortA ((REG_GPIO_PORT_A_T*)GPIOA)
#define PortB ((REG_GPIO_PORT_B_T*)GPIOB)
#define PortC ((REG_GPIO_PORT_C_T*)GPIOC)
#define PortD ((REG_GPIO_PORT_D_T*)GPIOD)
#define PortE ((REG_GPIO_PORT_E_T*)GPIOE)
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_X_GPIO_CONFIG_H_
