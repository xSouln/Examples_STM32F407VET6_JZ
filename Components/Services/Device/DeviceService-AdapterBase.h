//==============================================================================
#ifndef _DEVICE_SERVICE_ADAPTER_BASE_H_
#define _DEVICE_SERVICE_ADAPTER_BASE_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Common/xTypes.h"
//==============================================================================
/// @defgroup DeviceServiceBaseAdapter DeviceService adapter base types
/// @brief типы предостовляемые DeviceService-AdapterBase.h
/// @{

struct DeviceServiceT;
//------------------------------------------------------------------------------

typedef enum
{
	DeviceServiceAdapterEventIdle,
	
} DeviceServiceAdapterEventSelector;
//------------------------------------------------------------------------------

typedef enum
{
	DeviceServiceAdapterRequestIdle,

	DeviceServiceAdapterRequestsGet = 50,

} DeviceServiceAdapterRequestSelector;
//------------------------------------------------------------------------------

typedef void (*DeviceServiceAdapterHandlerT) (struct DeviceServiceT* service);
typedef void (*DeviceServiceAdapterEventListenerT) (struct DeviceServiceT* service, DeviceServiceAdapterEventSelector selector, void* arg);
typedef xResult (*DeviceServiceAdapterRequestListenerT) (struct DeviceServiceT* service, DeviceServiceAdapterRequestSelector selector, void* arg);
//------------------------------------------------------------------------------

typedef struct
{
	DeviceServiceAdapterHandlerT Handler;
	DeviceServiceAdapterRequestListenerT RequestListener;

} DeviceServiceAdapterInterfaceT;
//------------------------------------------------------------------------------

typedef struct
{
	char* Description;
	void* Content;

	DeviceServiceAdapterInterfaceT* Interface;

} DeviceServiceAdapterBaseT;
//------------------------------------------------------------------------------

typedef struct
{
	void* Content;

} DeviceServiceAdapterBaseInitT;

/// @}
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_DEVICE_SERVICE_ADAPTER_BASE_H_
