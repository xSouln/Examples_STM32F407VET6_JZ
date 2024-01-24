//==============================================================================
//header:

#ifndef _DEVICE_1_COMPONENT_CONFIG_H_
#define _DEVICE_1_COMPONENT_CONFIG_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Components-Types.h"
//==============================================================================
//macros:

#ifdef FREERTOS_USED
#define DEVICE1_COMPONENT_MAIN_TASK_STACK_SECTION __attribute__((section("._user_heap_stack")))
#endif

#define DEVICE1_LOCAL_SERVICES_MEM_SECTION __attribute__((section("._user_heap_stack")))
#define DEVICE1_MEM_SECTION __attribute__((section("._user_heap_stack")))

#define DEVICE1_PIN1_PORT PortE
#define DEVICE1_PIN1_HARDWARE_OTPUT 15
#define DEVICE1_PIN1_LOGIC_OTPUT 0

#define DEVICE_1_TRANSFER_LAYER &LocalTransferLayer
//==============================================================================
//import:


//==============================================================================
//defines:

#define DEVICE_1_PORT CAN_Port2
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_DEVICE_1_COMPONENT_CONFIG_H_
