//==============================================================================
//header:

#ifndef TCP_SERVER_TYPES_H
#define TCP_SERVER_TYPES_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Components_Types.h"
#include "TCPServer_Config.h"
#include "TCPServer_Info.h"

#include "Common/xTypes.h"
#include "Common/xTx.h"
#include "Common/xRx.h"

#include "W5500/w5500.h"
#include "WIZ/socket.h"

#include "TCPServer/Adapters/TCPServer_AdapterBase.h"
//==============================================================================
//types:

typedef enum
{
	TCPServerEventUpdateState = 1U,
	TCPServerEventEndLine,
	TCPServerEventReceiverBufferIsFull
	
} TCPServerEventSelector;
//------------------------------------------------------------------------------

typedef enum
{
	TCPServerRequestDelay = 1U,
	
} TCPServerRequestSelector;
//------------------------------------------------------------------------------

typedef enum
{
	TCPServerValueTransmitterStatus = 1U,
	
} TCPServerValueSelector;
//------------------------------------------------------------------------------

typedef union
{
  struct
	{
    uint32_t Connected : 1;
    uint32_t IsOpen : 1;
    uint32_t Update : 1;
  };
  uint32_t Value;
	
} TCPServerSockStatusT;
//------------------------------------------------------------------------------

typedef struct
{
  TCPServerSockStatusT Status;
  uint8_t Number;
  uint8_t State;
  uint16_t Port;
  uint16_t FreeSize;
	
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

typedef void (*TCPServerEventListenerT)(void* server, TCPServerEventSelector event, uint32_t args, uint32_t count);
typedef xResult (*TCPServerRequestListenerT)(void* server, TCPServerRequestSelector event, uint32_t args, uint32_t count);
//------------------------------------------------------------------------------

typedef struct
{
	TCPServerEventListenerT EventListener;
	TCPServerRequestListenerT RequestListener;
	
} TCPServerInterfaceT;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		xResult InitResult : 4;
		xResult AdapterInitResult : 4;
		xResult RxInitResult : 4;
		xResult TxInitResult : 4;
	};
	
	uint32_t Value;
	
} TCPServerStatusT;
//------------------------------------------------------------------------------

typedef struct
{
	OBJECT_HEADER;
	
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
#endif //TCP_SERVER_TYPES_H
