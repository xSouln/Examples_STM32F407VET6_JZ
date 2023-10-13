//==============================================================================
//header:

#ifndef _DEVICE_CONTROL_COMPONENT_H_
#define _DEVICE_CONTROL_COMPONENT_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components-Types.h"
#include "DeviceControl-ComponentConfig.h"
//==============================================================================
//defines:


//==============================================================================
//types:

typedef struct
{
	void* Parent;

} DeviceControlComponentInitT;
//==============================================================================
//functions:

xResult DeviceControlComponentInit(void* parent);
void DeviceControlComponentHandler();
void DeviceControlComponentTimeSynchronization();
//==============================================================================
//import:


//==============================================================================
//override:


//==============================================================================
//export:


//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_DEVICE_CONTROL_COMPONENT_H_
