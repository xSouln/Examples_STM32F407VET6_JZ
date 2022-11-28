//==============================================================================
//header:

#ifndef _TCP_CLIENT_ADAPTER_BASE_H
#define _TCP_CLIENT_ADAPTER_BASE_H
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
	TCPClientAdapterEventIdle,
	
	TCPClientAdapterEventUpdateTime

} TCPClientAdapterEventSelector;
//------------------------------------------------------------------------------

typedef enum
{
	TCPClientAdapterRequestIdle,
	
} TCPClientAdapterRequestSelector;
//------------------------------------------------------------------------------

typedef enum
{
	TCPClientAdapterValueIdle,
	
} TCPClientAdapterValueSelector;
//------------------------------------------------------------------------------
DEFINITION_HANDLER_TYPE(TCPClientAdapter);

DEFINITION_IRQ_LISTENER_TYPE(TCPClientAdapter);
DEFINITION_EVENT_LISTENER_TYPE(TCPClientAdapter, TCPClientAdapterEventSelector);
DEFINITION_REQUEST_LISTENER_TYPE(TCPClientAdapter, TCPClientAdapterRequestSelector);
//------------------------------------------------------------------------------

typedef struct
{
	DECLARE_HANDLER(TCPClientAdapter);

	DECLARE_IRQ_LISTENER(TCPClientAdapter);

	DECLARE_EVENT_LISTENER(TCPClientAdapter);
	DECLARE_REQUEST_LISTENER(TCPClientAdapter);
	
} TCPClientAdapterInterfaceT;
//------------------------------------------------------------------------------

typedef struct
{
	ObjectBaseT Object;
	
	void* Child;

	void* Register;

	TCPClientAdapterInterfaceT* Interface;
	
} TCPClientAdapterBaseT;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
