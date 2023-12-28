//==============================================================================
#ifndef _CAN_PORTS_COMPONENT_CONFIG_H_
#define _CAN_PORTS_COMPONENT_CONFIG_H_
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

#define CAN_PORT1_ENABLE 1
#define CAN_PORT2_ENABLE 1
#define CAN_PORT3_ENABLE 0
#define CAN_PORT4_ENABLE 0
#define CAN_PORT5_ENABLE 0
//------------------------------------------------------------------------------

enum
{
#if CAN_PORT1_ENABLE == 1
	CAN_PORT1,
#endif

#if CAN_PORT2_ENABLE == 1
	CAN_PORT2,
#endif

#if CAN_PORT3_ENABLE == 1
	CAN_PORT3,
#endif

#if CAN_PORT4_ENABLE == 1
	CAN_PORT4,
#endif

#if CAN_PORT5_ENABLE == 1
	CAN_PORT5,
#endif

#if CAN_PORT6_ENABLE == 1
	CAN_PORT6,
#endif

	CAN_PORTS_COUNT
};
//------------------------------------------------------------------------------

#define CAN_PORTS_MEM_SECTION //__attribute__((section("._user_heap_stack")))

#ifdef FREERTOS_USED

#define CAN_PORTS_MAIN_TASK_STACK_SECTION __attribute__((section("._user_heap_stack")))

#endif
//------------------------------------------------------------------------------
#if CAN_PORT1_ENABLE == 1
extern CAN_HandleTypeDef hcan1;

#define CAN_PORT1_HANDLE hcan1
#define CAN_PORT1_PORT_NUMBER xCAN1

#define CAN_PORT1_RX_CIRCLE_BUF_SIZE_MASK 0x3f
#define CAN_PORT1_TX_CIRCLE_BUF_SIZE_MASK 0x3f

#define CAN_PORT1_FILTER_BANK xCAN1
#define CAN_PORT1_FILTER_MODE CAN_FILTERMODE_IDMASK
#define CAN_PORT1_FILTER_SCALE CAN_FILTERSCALE_32BIT
#define CAN_PORT1_FILTER_ID_HIGH 0
#define CAN_PORT1_FILTER_ID_LOW 0
#define CAN_PORT1_FILTER_MASK_ID_HIGH 0
#define CAN_PORT1_FILTER_MASK_ID_LOW 0
#define CAN_PORT1_FILTER_RX_FIFO CAN_RX_FIFO0

#define CAN_PORT1_TX_CIRCLE_BUF_MEM_SECTION __attribute__((section("._user_heap_stack")))
#define CAN_PORT1_RX_CIRCLE_BUF_MEM_SECTION __attribute__((section("._user_heap_stack")))

#endif
//------------------------------------------------------------------------------

#if CAN_PORT2_ENABLE == 1
extern CAN_HandleTypeDef hcan2;

#define CAN_PORT2_HANDLE hcan2
#define CAN_PORT2_PORT_NUMBER xCAN2

#define CAN_PORT2_RX_CIRCLE_BUF_SIZE_MASK 0x3f
#define CAN_PORT2_TX_CIRCLE_BUF_SIZE_MASK 0x3f

#define CAN_PORT2_FILTER_BANK xCAN2
#define CAN_PORT2_FILTER_MODE CAN_FILTERMODE_IDMASK
#define CAN_PORT2_FILTER_SCALE CAN_FILTERSCALE_32BIT
#define CAN_PORT2_FILTER_ID_HIGH 0
#define CAN_PORT2_FILTER_ID_LOW 0
#define CAN_PORT2_FILTER_MASK_ID_HIGH 0
#define CAN_PORT2_FILTER_MASK_ID_LOW 0
#define CAN_PORT2_FILTER_RX_FIFO CAN_RX_FIFO0

#define CAN_PORT2_TX_CIRCLE_BUF_MEM_SECTION __attribute__((section("._user_heap_stack")))
#define CAN_PORT2_RX_CIRCLE_BUF_MEM_SECTION __attribute__((section("._user_heap_stack")))

#endif

//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_CAN_PORTS_COMPONENT_CONFIG_H_
