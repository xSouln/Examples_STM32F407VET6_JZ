//==============================================================================
//header:

#ifndef _HOST_REQUEST_CONTROL_COMPONENT_H_
#define _HOST_REQUEST_CONTROL_COMPONENT_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components-Types.h"
#include "CAN_Local/Control/CAN_Local-Types.h"
#include "Abstractions/xRequestControl/xRequestControl.h"
//==============================================================================
//defines:


//==============================================================================
//types:


//==============================================================================
//variables:


//==============================================================================
//functions:

xResult HostRequestControlComponentInit(void* parent);

void HostRequestControlComponentHandler();

#define HostRequestControlComponentTimeSynchronization()
//==============================================================================
//import:


//==============================================================================
//override:


//==============================================================================
//export:

extern xRequestControlT HostRequestControl;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_HOST_REQUEST_CONTROL_COMPONENT_H_
