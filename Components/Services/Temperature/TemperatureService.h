//==============================================================================
#ifndef _TEMPERATURE_SERVICE_H_
#define _TEMPERATURE_SERVICE_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xDevice/xService.h"
#include "TemperatureService-AdapterBase.h"
//==============================================================================
//includes:

#define TEMPERATURE_SERVICE_UID 0x5C78700
//==============================================================================
/// @defgroup xServices temperature service types
/// @brief функции предостовляемые TemperatureService.c
/// @{

typedef enum
{
	TemperatureServiceRequestIdle = xServiceBaseRequestOffset,

	TemperatureServiceRequestGetTemperature

} TemperatureServiceRequests;
//------------------------------------------------------------------------------

typedef struct
{
	xServiceT Base;

	TemperatureServiceAdapterBaseT Adapter;

	float Temperature;

} TemperatureServiceT;
//------------------------------------------------------------------------------

typedef struct
{
	xServiceInitT Base;

} TemperatureServiceInitT;
/// @}
//==============================================================================
/// @defgroup xServices temperature service functions
/// @brief функции предостовляемые слоем TemperatureService.c
/// @{

xResult TemperatureServiceInit(TemperatureServiceT* service, TemperatureServiceInitT* init);

/// @}
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TEMPERATURE_SERVICE_H_
