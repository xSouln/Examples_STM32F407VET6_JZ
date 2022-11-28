//==============================================================================
//module enable:

#include "TCPServer/Adapters/TCPServer_AdapterConfig.h"
#ifdef TCP_SERVER_LWIP_ADAPTER_ENABLE
//==============================================================================
//header:

#ifndef _TCP_SERVER_LWIP_ADAPTER_H
#define _TCP_SERVER_LWIP_ADAPTER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "TCPServer/Controls/TCPServer.h"
#include "Common/xRxReceiver.h"
#include "lwip.h"
#include "lwip/tcp.h"
//==============================================================================
//types:

//------------------------------------------------------------------------------

typedef struct
{
	struct tcp_pcb* Socket;
	struct tcp_pcb* SocketHandle;

	uint16_t OtputUpdateTime;

} TCPServerLWIPAdapterDataT;
//------------------------------------------------------------------------------

typedef struct
{
	TCPServerLWIPAdapterDataT Data;

	ETH_HandleTypeDef* Handle;

	xDataBufferT* ResponseBuffer;
	xRxReceiverT RxReceiver;

	xCircleBufferT RxCircleBuffer;

} TCPServerLWIPAdapterT;
//==============================================================================
//functions:

xResult TCPServerLWIPAdapterInit(TCPServerT* server, TCPServerLWIPAdapterT* adapter);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TCP_SERVER_LWIP_ADAPTER_H
#endif //TCP_SERVER_LWIP_ADAPTER_ENABLE
