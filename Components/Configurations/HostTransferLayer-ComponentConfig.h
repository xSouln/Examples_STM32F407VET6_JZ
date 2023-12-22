//==============================================================================
//header:

#ifndef _TRANSFER_LAYER_COMPONENT_CONFIG_H_
#define _TRANSFER_LAYER_COMPONENT_CONFIG_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Components-Types.h"
//==============================================================================
//macros:


//==============================================================================
//import:


//==============================================================================
//defines:

#define HOST_TRANSFER_LAYER_BUFFER_MEM_SECTION __attribute__((section("._user_heap_stack")))

#define HOST_TRANSFER_LAYER_BUFFER_SIZE 20
#define HOST_TRANSFER_LAYER_PORT CAN_Port1
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TRANSFER_LAYER_COMPONENT_CONFIG_H_
