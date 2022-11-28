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

#include "Components_Types.h"
//==============================================================================
//types:

typedef enum
{
	TCPServerAdapterEventIdle,
	
	TCPServerAdapterEventUpdateTime

} TCPServerAdapterEventSelector;
//------------------------------------------------------------------------------

typedef enum
{
	TCPServerAdapterRequestIdle,
	
} TCPServerAdapterRequestSelector;
//------------------------------------------------------------------------------

typedef enum
{
	TCPServerAdapterValueIdle,
	
} TCPServerAdapterValueSelector;
//------------------------------------------------------------------------------
DEFINITION_HANDLER_TYPE(TCPServerAdapter);

DEFINITION_IRQ_LISTENER_TYPE(TCPServerAdapter);
DEFINITION_EVENT_LISTENER_TYPE(TCPServerAdapter, TCPServerAdapterEventSelector);
DEFINITION_REQUEST_LISTENER_TYPE(TCPServerAdapter, TCPServerAdapterRequestSelector);
//------------------------------------------------------------------------------

typedef struct
{
	DECLARE_HANDLER(TCPServerAdapter);

	DECLARE_IRQ_LISTENER(TCPServerAdapter);

	DECLARE_EVENT_LISTENER(TCPServerAdapter);
	DECLARE_REQUEST_LISTENER(TCPServerAdapter);
	
} TCPServerAdapterInterfaceT;
//------------------------------------------------------------------------------

typedef struct
{
	ObjectBaseT Object;
	
	void* Child;

	void* Register;

	TCPServerAdapterInterfaceT* Interface;
	
} TCPServerAdapterBaseT;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
