//==============================================================================
//header:

#ifndef _TCP_SERVER_ADAPTER_BASE_H
#define _TCP_SERVER_ADAPTER_BASE_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Common/xTypes.h"
//==============================================================================
//types:

typedef enum
{
	TCPServerAdapterTxIRQ = 1U,
	TCPServerAdapterEventEndLine,
	TCPServerAdapterEventReceiverBufferIsFull
	
} TCPServerAdapterEventSelector;
//------------------------------------------------------------------------------

typedef enum
{
	TCPServerAdapterRequestDelay = 1U,
	
} TCPServerAdapterRequestSelector;
//------------------------------------------------------------------------------

typedef enum
{
	TCPServerAdapterValueTransmitterStatus = 1U,
	
} TCPServerAdapterValueSelector;
//------------------------------------------------------------------------------

typedef void (*TCPServerAdapterHandlerT)(void* device);

typedef void (*TCPServerAdapterEventListenerT)(void* device, TCPServerAdapterEventSelector event, uint32_t args, uint32_t count);
typedef xResult (*TCPServerAdapterRequestListenerT)(void* device, TCPServerAdapterRequestSelector event, uint32_t args, uint32_t count);

typedef uint32_t (*TCPServerAdapterActionGetValueT)(void* device, TCPServerAdapterValueSelector event);
typedef xResult (*TCPServerAdapterActionSetValueT)(void* device,TCPServerAdapterValueSelector event, uint32_t value);
//------------------------------------------------------------------------------

typedef struct
{
	TCPServerAdapterHandlerT Handler;
	
	TCPServerAdapterEventListenerT EventListener;
	TCPServerAdapterRequestListenerT RequestListener;
	
	TCPServerAdapterActionGetValueT GetValue;
	TCPServerAdapterActionSetValueT SetValue;
	
} TCPServerAdapterInterfaceT;
//------------------------------------------------------------------------------

typedef struct
{
	OBJECT_HEADER;
	
	void* Child;

	TCPServerAdapterInterfaceT* Interface;
	
} TCPServerAdapterBaseT;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
