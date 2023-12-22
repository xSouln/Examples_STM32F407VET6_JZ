//==============================================================================
//header:

#ifndef _HOST_REQUEST_CONTROL_COMPONENT_CONFIG_H_
#define _HOST_REQUEST_CONTROL_COMPONENT_CONFIG_H_
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

#define HOST_REQUEST_CONTROL_BUFFER_MEM_SECTION __attribute__((section("._user_heap_stack")))

#define HOST_REQUEST_CONTROL_BUFFER_SIZE 20
#define HOST_REQUEST_CONTROL_PORT CAN_Port
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_HOST_REQUEST_CONTROL_COMPONENT_CONFIG_H_
