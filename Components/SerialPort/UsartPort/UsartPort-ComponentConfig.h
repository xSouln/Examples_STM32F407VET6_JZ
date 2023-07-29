//==============================================================================
#ifndef _USART_PORT_COMPONENT_CONFIG_H_
#define _USART_PORT_COMPONENT_CONFIG_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components-Types.h"
//==============================================================================
//import:

extern DMA_HandleTypeDef hdma_usart3_rx;
//==============================================================================
//defines:

#define SERIAL1_ENABLE
//------------------------------------------------------------------------------

enum
{
#ifdef SERIAL1_ENABLE
	SERIAL1,
#endif

#ifdef SERIAL2_ENABLE
	SERIAL2,
#endif

#ifdef SERIAL3_ENABLE
	SERIAL3,
#endif

#ifdef SERIAL4_ENABLE
	SERIAL4,
#endif

#ifdef SERIAL5_ENABLE
	SERIAL5,
#endif

	SERIAL_PORTS_COUNT
};
//------------------------------------------------------------------------------

#ifdef SERIAL1_ENABLE
#define SERIAL1_RX_CIRCLE_BUF_SIZE_MASK 0x1ff
#define SERIAL1_RX_OBJECT_BUF_SIZE 0x1ff
#define SERIAL1_TX_CIRCLE_BUF_SIZE_MASK 0x3ff
#define SERIAL1_REG USART3
#define SERIAL1_RX_DMA hdma_usart3_rx
#endif

#ifdef SERIAL2_ENABLE
#define SERIAL2_RX_CIRCLE_BUF_SIZE_MASK 0x1ff
#define SERIAL2_RX_OBJECT_BUF_SIZE 0x1ff
#define SERIAL2_TX_CIRCLE_BUF_SIZE_MASK 0x3ff
#define SERIAL2_REG USART3
#define SERIAL2_RX_DMA hdma_usart3_rx
#endif

#ifdef SERIAL3_ENABLE
#define SERIAL3_RX_CIRCLE_BUF_SIZE_MASK 0x1ff
#define SERIAL3_RX_OBJECT_BUF_SIZE 0x1ff
#define SERIAL3_TX_CIRCLE_BUF_SIZE_MASK 0x3ff
#define SERIAL3_REG USART3
#define SERIAL3_RX_DMA hdma_usart3_rx
#endif

#ifdef SERIAL4_ENABLE
#define SERIAL4_RX_CIRCLE_BUF_SIZE_MASK 0x1ff
#define SERIAL4_RX_OBJECT_BUF_SIZE 0x1ff
#define SERIAL4_TX_CIRCLE_BUF_SIZE_MASK 0x3ff
#define SERIAL4_REG USART3
#define SERIAL4_RX_DMA hdma_usart3_rx
#endif

//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_USART_PORT_COMPONENT_CONFIG_H_
