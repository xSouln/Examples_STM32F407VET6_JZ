//==============================================================================
#ifndef _GAP_SERVICE_ADAPTER_BASE_H_
#define _GAP_SERVICE_ADAPTER_BASE_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Common/xTypes.h"
//==============================================================================
/// @defgroup GAPServiceBaseAdapter GAPService adapter base types
/// @brief типы предостовляемые GAPService-AdapterBase.h
/// @{

struct GAPServiceT;
//------------------------------------------------------------------------------

typedef enum
{
	GAPServiceAdapterEventIdle,
	
} GAPServiceAdapterEventSelector;
//------------------------------------------------------------------------------

typedef enum
{
	GAPServiceAdapterRequestIdle,

	GAPServiceAdapterRequestDispose,

} GAPServiceAdapterRequestSelector;
//------------------------------------------------------------------------------

typedef void (*GAPServiceAdapterHandlerT) (struct GAPServiceT* service);
typedef void (*GAPServiceAdapterEventListenerT) (struct GAPServiceT* service, GAPServiceAdapterEventSelector selector, void* arg);
typedef xResult (*GAPServiceAdapterRequestListenerT) (struct GAPServiceT* service, GAPServiceAdapterRequestSelector selector, void* arg);
//------------------------------------------------------------------------------

typedef struct
{
	GAPServiceAdapterHandlerT Handler;
	GAPServiceAdapterRequestListenerT RequestListener;

} GAPServiceAdapterInterfaceT;
//------------------------------------------------------------------------------

typedef struct
{
	GAPServiceAdapterInterfaceT* Interface;

} GAPServiceAdapterBaseT;

/// @}
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_GAP_SERVICE_ADAPTER_BASE_H_
