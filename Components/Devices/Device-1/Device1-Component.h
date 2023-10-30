//==============================================================================
//header:

#ifndef _DEVICE_1_COMPONENT_H_
#define _DEVICE_1_COMPONENT_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components-Types.h"
#include "Abstractions/xDevice/xDevice.h"
#include "Services/Temperature/TemperatureService.h"
//==============================================================================
//defines:

#define TEMPERATURE_SERVICE3_ID 2022
#define TEMPERATURE_SERVICE4_ID 2023
//==============================================================================
//functions:

xResult Device1ComponentInit(void* parent);
void Device1ComponentHandler();
void Device1ComponentTimeSynchronization();
//==============================================================================
//import:


//==============================================================================
//override:


//==============================================================================
//export:

extern xDeviceT Device1;
extern TemperatureServiceT TemperatureService3;
extern TemperatureServiceT TemperatureService4;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_DEVICE_1_COMPONENT_H_
