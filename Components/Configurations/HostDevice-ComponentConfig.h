//==============================================================================
//header:

#ifndef _HOST_DEVICE_COMPONENT_CONFIG_H_
#define _HOST_DEVICE_COMPONENT_CONFIG_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Components-Types.h"
#include "HostDevice-ComponentConfig.h"
//==============================================================================
//macros:

#ifdef FREERTOS_USED
#define HOST_DEVICE_MAIN_TASK_STACK_SIZE 0x100
#define HOST_DEVICE_MAIN_TASK_STACK_SECTION __attribute__((section("._user_heap_stack")))
#endif

#define HOST_DEVICE_LOCAL_SERVICES_MEM_SECTION __attribute__((section("._user_heap_stack")))
#define HOST_DEVICE_MEM_SECTION __attribute__((section("._user_heap_stack"))) = { 0 }
//==============================================================================
//import:


//==============================================================================
//defines:

#define HOST_DEVICE_PORT CAN_Port1
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_HOST_DEVICE_COMPONENT_CONFIG_H_
