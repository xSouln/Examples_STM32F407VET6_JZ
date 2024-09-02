//==============================================================================
//header:

#ifndef _NET_COMPONENT_CONFIG_H_
#define _NET_COMPONENT_CONFIG_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Components-Types.h"
//==============================================================================
//defines:

#ifdef INC_FREERTOS_H

#ifndef NET_TASK_STACK_SIZE
#define NET_TASK_STACK_SIZE 0x100
#endif

#ifndef NET_COMPONENT_MAIN_TASK_STACK_SECTION
#define NET_COMPONENT_MAIN_TASK_STACK_SECTION __attribute__((section("._user_heap_stack")))
#endif

#endif //INC_FREERTOS_H
//------------------------------------------------------------------------------
#define NET_UNDEFINED_LAYOUT 0
#define NET_LWIP_LAYOUT 1
#define NET_FREERTOS_LAYOUT 2

#ifndef NET_TARGET_LAYOUT
#define NET_TARGET_LAYOUT NET_FREERTOS_LAYOUT
#endif

#define NET_RX_BUFFER_MEM_SECTION __attribute__((section("._user_heap_stack")))
#define NET_RX_OPERATION_BUFFER_MEM_SECTION
#define NET_TX_BUFFER_MEM_SECTION
#define NET_MEM_SECTION __attribute__((section("._user_heap_stack")))
#define NET_PORT_MEM_SECTION __attribute__((section("._user_heap_stack")))

#ifndef NET_RX_OPERATION_BUFFER_SIZE
#define NET_RX_OPERATION_BUFFER_SIZE 0x200
#endif

#ifndef NET_RX_BUFFER_SIZE
#define NET_RX_BUFFER_SIZE 0x200
#endif

#ifndef NET_TX_BUFFER_SIZE
#define NET_TX_BUFFER_SIZE 0x400
#endif

#ifndef NET_PORT_START_ID_OFFSET
#define NET_PORT_START_ID_OFFSET 10
#endif
//==============================================================================
//import:


//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_NET_COMPONENT_CONFIG_H_
