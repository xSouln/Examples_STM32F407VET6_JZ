//==============================================================================
//module enable:

#include "TCPClient/Adapters/TCPClient_AdapterConfig.h"
#ifdef TCP_CLIENT_LWIP_ADAPTER_ENABLE
//==============================================================================
//header:

#ifndef _TCP_CLIENT_LWIP_ADAPTER_H
#define _TCP_CLIENT_LWIP_ADAPTER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "TCPClient/Controls/TCPClient.h"
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

} TCPClientLWIPAdapterDataT;
//------------------------------------------------------------------------------

typedef struct
{
	TCPClientLWIPAdapterDataT Data;

	ETH_HandleTypeDef* Handle;

	xDataBufferT* ResponseBuffer;
	xRxReceiverT RxReceiver;

	xCircleBufferT RxCircleBuffer;

} TCPClientLWIPAdapterT;
//==============================================================================
//functions:

xResult TCPClientLWIPAdapterInit(TCPClientT* server, TCPClientLWIPAdapterT* adapter);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TCP_CLIENT_LWIP_ADAPTER_H
#endif //TCP_CLIENT_LWIP_ADAPTER_ENABLE
