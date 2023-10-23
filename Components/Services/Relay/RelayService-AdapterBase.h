//==============================================================================
#ifndef _RELAY_SERVICE_ADAPTER_BASE_H_
#define _RELAY_SERVICE_ADAPTER_BASE_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Common/xTypes.h"
//==============================================================================
/// @defgroup RelayServiceBaseAdapter RelayService adapter base types
/// @brief типы предостовляемые RelayService-AdapterBase.h
/// @{

struct RelayServiceT;
//------------------------------------------------------------------------------

typedef enum
{
	RelayServiceAdapterEventIdle,
	
} RelayServiceAdapterEventSelector;
//------------------------------------------------------------------------------

typedef enum
{
	RelayServiceAdapterRequestIdle,

	RelayServiceAdapterRequestsGet = 50,
	RelayServiceAdapterRequestGetRelay

} RelayServiceAdapterRequestSelector;
//------------------------------------------------------------------------------

typedef void (*RelayServiceAdapterHandlerT) (struct RelayServiceT* service);
typedef void (*RelayServiceAdapterEventListenerT) (struct RelayServiceT* service, RelayServiceAdapterEventSelector selector, void* arg);
typedef xResult (*RelayServiceAdapterRequestListenerT) (struct RelayServiceT* service, RelayServiceAdapterRequestSelector selector, void* arg);
//------------------------------------------------------------------------------

typedef struct
{
	RelayServiceAdapterHandlerT Handler;
	RelayServiceAdapterRequestListenerT RequestListener;
	RelayServiceAdapterEventListenerT EventListener;

} RelayServiceAdapterInterfaceT;
//------------------------------------------------------------------------------

typedef struct
{
	char* Description;
	void* Content;

	RelayServiceAdapterInterfaceT* Interface;

} RelayServiceAdapterBaseT;
//------------------------------------------------------------------------------

typedef struct
{
	void* Content;

} RelayServiceAdapterBaseInitT;

/// @}
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_RELAY_SERVICE_ADAPTER_BASE_H_
