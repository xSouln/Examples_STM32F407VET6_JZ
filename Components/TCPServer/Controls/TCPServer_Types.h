//==============================================================================
//header:

#ifndef _TCP_SERVER_TYPES_H
#define _TCP_SERVER_TYPES_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Components_Types.h"
#include "TCPServer_Info.h"
#include "TCPServer_Config.h"

#include "Common/xTx.h"
#include "Common/xRx.h"

#include "TCPServer/Adapters/TCPServer_AdapterBase.h"
//==============================================================================
//types:

typedef enum
{
	TCPServerEventIdle,

	TCPServerEventUpdateState,
	TCPServerEventEndLine,
	TCPServerEventBufferIsFull
	
} TCPServerEventSelector;
//------------------------------------------------------------------------------

typedef enum
{
	TCPServerRequestIdle,

	TCPServerRequestDelay,
	
} TCPServerRequestSelector;
//------------------------------------------------------------------------------

typedef enum
{
	TCPServerValueIdle,

	TCPServerValueTransmitterStatus,
	
} TCPServerValueSelector;
//------------------------------------------------------------------------------

typedef enum
{
	TCPServerSockDisconnect,
	TCPServerSockConnecting,
	TCPServerSockConnected,
	TCPServerSockDisconnecting,

} TCPServerSockStates;
//------------------------------------------------------------------------------

typedef enum
{
	TCPServerSockErrorNo,
	TCPServerSockErrorAborted,

} TCPServerSockErrors;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		TCPServerSockStates State;
		TCPServerSockErrors Error;
	};

	uint32_t Value;
	
} TCPServerSockStatusT;
//------------------------------------------------------------------------------

typedef struct
{
	TCPServerSockStatusT Status;

	uint8_t Number;
	uint16_t Port;
	
	void* Handle;

} TCPServerSockT;
//------------------------------------------------------------------------------

typedef struct
{
	uint8_t Ip[4];
	uint8_t Gateway[4];
	uint8_t NetMask [4];
	uint8_t Mac[6];
	
} TCPServerInfoT;
//------------------------------------------------------------------------------

typedef struct
{
	uint8_t* Data;
	uint32_t Size;

} TCPServerReceivedDataT;
//------------------------------------------------------------------------------

DEFINITION_EVENT_LISTENER_TYPE(TCPServer, TCPServerEventSelector);
DEFINITION_REQUEST_LISTENER_TYPE(TCPServer, TCPServerRequestSelector);
//------------------------------------------------------------------------------

typedef struct
{
	DECLARE_EVENT_LISTENER(TCPServer);
	DECLARE_REQUEST_LISTENER(TCPServer);
	
} TCPServerInterfaceT;
//------------------------------------------------------------------------------


typedef enum
{
	TCPServerClosed,
	TCPServerOpening,
	TCPServerIsOpen,
	TCPServerClosing,

} TCPServerStates;
//------------------------------------------------------------------------------

typedef enum
{
	TCPServerErrorNo,
	TCPServerErrorAborted,

} TCPServerErrors;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		TCPServerStates State;
		TCPServerErrors Error;
	};
	
	uint32_t Value;
	
} TCPServerStatusT;
//------------------------------------------------------------------------------

typedef struct
{
	ObjectBaseT Object;
	
	TCPServerAdapterBaseT Adapter;
	
	TCPServerInterfaceT* Interface;
	
	TCPServerSockT Sock;
	TCPServerInfoT Options;
	TCPServerInfoT Info;
	
	TCPServerStatusT Status;

	xTxT Tx;
	xRxT Rx;
	  
} TCPServerT;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TCP_SERVER_TYPES_H
