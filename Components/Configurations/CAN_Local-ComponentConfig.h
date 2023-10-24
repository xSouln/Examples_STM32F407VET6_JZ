//==============================================================================
#ifndef _CAN_LOCAL_COMPONENT_CONFIG_H_
#define _CAN_LOCAL_COMPONENT_CONFIG_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components-Types.h"
#include "Peripherals/CAN/xCAN.h"
//==============================================================================
//import:

//==============================================================================
//defines:

#define CAN_LOCAL1_ENABLE
#define CAN_LOCAL2_ENABLE
//#define CAN_LOCAL3_ENABLE
//#define CAN_LOCAL4_ENABLE
//#define CAN_LOCAL5_ENABLE
//------------------------------------------------------------------------------

enum
{
#ifdef CAN_LOCAL1_ENABLE
	CAN_LOCAL1,
#endif

#ifdef CAN_LOCAL2_ENABLE
	CAN_LOCAL2,
#endif

#ifdef CAN_LOCAL3_ENABLE
	CAN_LOCAL3,
#endif

#ifdef CAN_LOCAL4_ENABLE
	CAN_LOCAL4,
#endif

#ifdef CAN_LOCAL5_ENABLE
	CAN_LOCAL5,
#endif

#ifdef CAN_LOCAL6_ENABLE
	CAN_LOCAL6,
#endif

	CAN_LOCAL_PORTS_COUNT
};
//------------------------------------------------------------------------------

#ifdef CAN_LOCAL1_ENABLE
extern CAN_HandleTypeDef hcan1;

#define CAN_LOCAL1_HANDLE hcan1
#define CAN_LOCAL1_PORT_NUMBER xCAN1

#define CAN_LOCAL1_RX_CIRCLE_BUF_SIZE_MASK 0x3f
#define CAN_LOCAL1_TX_CIRCLE_BUF_SIZE_MASK 0x3f

#define CAN_LOCAL1_FILTER_BANK xCAN1
#define CAN_LOCAL1_FILTER_MODE CAN_FILTERMODE_IDMASK
#define CAN_LOCAL1_FILTER_SCALE CAN_FILTERSCALE_32BIT
#define CAN_LOCAL1_FILTER_ID_HIGH 0
#define CAN_LOCAL1_FILTER_ID_LOW 0
#define CAN_LOCAL1_FILTER_MASK_ID_HIGH 0
#define CAN_LOCAL1_FILTER_MASK_ID_LOW 0
#define CAN_LOCAL1_FILTER_RX_FIFO CAN_RX_FIFO0
#endif
//------------------------------------------------------------------------------

#ifdef CAN_LOCAL2_ENABLE
extern CAN_HandleTypeDef hcan2;

#define CAN_LOCAL2_HANDLE hcan2
#define CAN_LOCAL2_PORT_NUMBER xCAN2

#define CAN_LOCAL2_RX_CIRCLE_BUF_SIZE_MASK 0x3f
#define CAN_LOCAL2_TX_CIRCLE_BUF_SIZE_MASK 0x3f

#define CAN_LOCAL2_FILTER_BANK xCAN2
#define CAN_LOCAL2_FILTER_MODE CAN_FILTERMODE_IDMASK
#define CAN_LOCAL2_FILTER_SCALE CAN_FILTERSCALE_32BIT
#define CAN_LOCAL2_FILTER_ID_HIGH 0
#define CAN_LOCAL2_FILTER_ID_LOW 0
#define CAN_LOCAL2_FILTER_MASK_ID_HIGH 0
#define CAN_LOCAL2_FILTER_MASK_ID_LOW 0
#define CAN_LOCAL2_FILTER_RX_FIFO CAN_RX_FIFO0
#endif

//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_CAN_LOCAL_COMPONENT_CONFIG_H_
