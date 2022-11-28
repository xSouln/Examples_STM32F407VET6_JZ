//==============================================================================
//header:

#ifndef _TCP_CLIENT_TYPES_H
#define _TCP_CLIENT_TYPES_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Components_Types.h"
#include "TCPClient_Info.h"
#include "TCPClient_Config.h"

#include "Common/xTx.h"
#include "Common/xRx.h"

#include "TCPClient/Adapters/TCPClient_AdapterBase.h"
//==============================================================================
//types:

typedef enum
{
	TCPClientEventIdle,

	TCPClientEventUpdateState,
	TCPClientEventEndLine,
	TCPClientEventBufferIsFull
	
} TCPClientEventSelector;
//------------------------------------------------------------------------------

typedef enum
{
	TCPClientRequestIdle,

	TCPClientRequestDelay,
	
} TCPClientRequestSelector;
//------------------------------------------------------------------------------

typedef enum
{
	TCPClientValueIdle,

	TCPClientValueTransmitterStatus,
	
} TCPClientValueSelector;
//------------------------------------------------------------------------------

typedef enum
{
	TCPClientSockDisconnect,
	TCPClientSockConnecting,
	TCPClientSockConnected,
	TCPClientSockDisconnecting,

} TCPClientSockStates;
//------------------------------------------------------------------------------

typedef enum
{
	TCPClientSockErrorNo,
	TCPClientSockErrorAborted,

} TCPClientSockErrors;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		TCPClientSockStates State;
		TCPClientSockErrors Error;
	};

	uint32_t Value;
	
} TCPClientSockStatusT;
//------------------------------------------------------------------------------

typedef struct
{
	TCPClientSockStatusT Status;

	uint8_t Number;
	uint16_t Port;
	
	void* Handle;

} TCPClientSockT;
//------------------------------------------------------------------------------

typedef struct
{
	uint8_t Ip[4];
	uint8_t Gateway[4];
	uint8_t NetMask [4];
	uint8_t Mac[6];
	
} TCPClientInfoT;
//------------------------------------------------------------------------------

typedef struct
{
	uint8_t* Data;
	uint32_t Size;

} TCPClientReceivedDataT;
//------------------------------------------------------------------------------

DEFINITION_EVENT_LISTENER_TYPE(TCPClient, TCPClientEventSelector);
DEFINITION_REQUEST_LISTENER_TYPE(TCPClient, TCPClientRequestSelector);
//------------------------------------------------------------------------------

typedef struct
{
	DECLARE_EVENT_LISTENER(TCPClient);
	DECLARE_REQUEST_LISTENER(TCPClient);
	
} TCPClientInterfaceT;
//------------------------------------------------------------------------------


typedef enum
{
	TCPClientClosed,
	TCPClientOpening,
	TCPClientIsOpen,
	TCPClientClosing,

} TCPClientStates;
//------------------------------------------------------------------------------

typedef enum
{
	TCPClientErrorNo,
	TCPClientErrorAborted,

} TCPClientErrors;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		TCPClientStates State;
		TCPClientErrors Error;
	};
	
	uint32_t Value;
	
} TCPClientStatusT;
//------------------------------------------------------------------------------

typedef struct
{
	ObjectBaseT Object;
	
	TCPClientAdapterBaseT Adapter;
	
	TCPClientInterfaceT* Interface;
	
	TCPClientSockT Sock;
	TCPClientInfoT Options;
	TCPClientInfoT Info;
	
	TCPClientStatusT Status;

	xTxT Tx;
	xRxT Rx;
	  
} TCPClientT;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TCP_CLIENT_TYPES_H
