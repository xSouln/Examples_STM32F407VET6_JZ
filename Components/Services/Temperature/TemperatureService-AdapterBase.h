//==============================================================================
#ifndef _TEMPERATURE_SERVICE_ADAPTER_BASE_H_
#define _TEMPERATURE_SERVICE_ADAPTER_BASE_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Common/xTypes.h"
//==============================================================================
/// @defgroup TemperatureServiceBaseAdapter TemperatureService adapter base types
/// @brief типы предостовляемые TemperatureService-AdapterBase.h
/// @{

struct TemperatureServiceT;
//------------------------------------------------------------------------------

typedef enum
{
	TemperatureServiceAdapterEventIdle,
	
} TemperatureServiceAdapterEventSelector;
//------------------------------------------------------------------------------

typedef enum
{
	TemperatureServiceAdapterRequestIdle,

	TemperatureServiceAdapterRequestsGet = 50,
	TemperatureServiceAdapterRequestGetTemperature

} TemperatureServiceAdapterRequestSelector;
//------------------------------------------------------------------------------

typedef void (*TemperatureServiceAdapterHandlerT) (struct TemperatureServiceT* service);
typedef void (*TemperatureServiceAdapterEventListenerT) (struct TemperatureServiceT* service, TemperatureServiceAdapterEventSelector selector, void* arg);
typedef xResult (*TemperatureServiceAdapterRequestListenerT) (struct TemperatureServiceT* service, TemperatureServiceAdapterRequestSelector selector, void* arg);
//------------------------------------------------------------------------------

typedef struct
{
	TemperatureServiceAdapterHandlerT Handler;
	TemperatureServiceAdapterRequestListenerT RequestListener;
	TemperatureServiceAdapterEventListenerT EventListener;

} TemperatureServiceAdapterInterfaceT;
//------------------------------------------------------------------------------

typedef struct
{
	char* Description;
	void* Content;

	TemperatureServiceAdapterInterfaceT* Interface;

} TemperatureServiceAdapterBaseT;
//------------------------------------------------------------------------------

typedef struct
{
	void* Content;

} TemperatureServiceAdapterBaseInitT;

/// @}
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TEMPERATURE_SERVICE_ADAPTER_BASE_H_
