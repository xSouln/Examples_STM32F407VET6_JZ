//==============================================================================
#ifndef _DEVICE_SERVICE_H_
#define _DEVICE_SERVICE_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xDevice/xService.h"
#include "DeviceService-AdapterBase.h"
//==============================================================================
//includes:

#define TEMPERATURE_SERVICE_UID 0x5C78700
//==============================================================================
/// @defgroup xServices temperature service types
/// @brief функции предостовляемые DeviceService.c
/// @{

typedef enum
{
	DeviceServiceRequestIdle = xServiceBaseRequestOffset,

	DeviceServiceRequestGetTemperature

} DeviceServiceRequests;
//------------------------------------------------------------------------------

typedef struct DeviceServiceT
{
	xServiceT Base;

	DeviceServiceAdapterBaseT Adapter;

} DeviceServiceT;
//------------------------------------------------------------------------------

typedef struct
{
	xServiceInitT Base;

} DeviceServiceInitT;
/// @}
//==============================================================================
/// @defgroup xServices temperature service functions
/// @brief функции предостовляемые слоем DeviceService.c
/// @{

xResult DeviceServiceInit(DeviceServiceT* service, DeviceServiceInitT* init);

/// @}
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_DEVICE_SERVICE_H_
