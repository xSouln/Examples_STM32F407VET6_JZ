//==============================================================================
//module enable:

#include "TCPServer/TCPServer_ComponentConfig.h"
#ifdef TCP_SERVER_LWIP_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "TCPServer_LWIP_Component.h"
#include "TCPServer/TCPServer_Component.h"
#include "TCPServer/Adapters/LWIP/TCPServer_LWIP_Adapter.h"

#include "Components.h"

#ifdef TERMINAL_COMPONENT_ENABLE
#include "Terminal/Controls/Terminal.h"
#endif
//==============================================================================
//defines:

#define RX_BUF_SIZE TCP_SERVER_LWIP_RX_BUF_SIZE
#define RX_RECEIVER_BUF_SIZE TCP_SERVER_LWIP_RX_RECEIVER_BUF_SIZE
//==============================================================================
//variables:

static uint8_t rx_buf[RX_BUF_SIZE];
static uint8_t rx_receiver_buf[RX_RECEIVER_BUF_SIZE];

TCPServerT TCPServerLWIP;
//==============================================================================
//functions:

static void _TCPServerLWIPComponentEventListener(TCPServerT* server, TCPServerEventSelector selector, void* arg, ...)
{
	switch ((uint8_t)selector)
	{
		case TCPServerEventEndLine:
			break;
		
		case TCPServerEventBufferIsFull:
			break;
	}
}
//------------------------------------------------------------------------------
static xResult _TCPServerLWIPComponentRequestListener(TCPServerT* server, TCPServerRequestSelector selector, void* arg, ...)
{
	switch ((uint8_t)selector)
	{
		case TCPServerRequestDelay:
			HAL_Delay((uint32_t)arg);
			break;
		
		default : return xResultRequestIsNotFound;
	}
	
	return xResultAccept;
}
//------------------------------------------------------------------------------

void _TCPServerLWIPComponentIRQListener()
{
	TCPServerIRQListener(&TCPServerLWIP);
}
//------------------------------------------------------------------------------
/**
 * @brief main handler
 */
void _TCPServerLWIPComponentHandler()
{
	TCPServerHandler(&TCPServerLWIP);
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
void _TCPServerLWIPComponentTimeSynchronization()
{
	TCPServerTimeSynchronization(&TCPServerLWIP);
}
//==============================================================================
//initializations:

static TCPServerInterfaceT TCPServerInterface =
{
	INITIALIZATION_EVENT_LISTENER(TCPServer, ComponentsEventListener),
	INITIALIZATION_REQUEST_LISTENER(TCPServer, ComponentsRequestListener)
};

//------------------------------------------------------------------------------

static TCPServerLWIPAdapterT TCPServerLWIPAdapter =
{
	.Handle = &heth
};
//==============================================================================
//initialization:

xResult _TCPServerLWIPComponentInit(void* parent)
{
	#ifdef TERMINAL_COMPONENT_ENABLE
	TCPServerLWIPAdapter.ResponseBuffer = &Terminal.ResponseBuffer;
	#endif
	
	TCPServerLWIPAdapter.RxBuffer = rx_buf;
	TCPServerLWIPAdapter.RxBufferSize = sizeof(rx_buf);
	
	xRxReceiverInit(&TCPServerLWIPAdapter.RxReceiver,
									&TCPServerLWIP.Rx,
									0,
									rx_receiver_buf,
									sizeof(rx_receiver_buf));
	
	TCPServerLWIPAdapterInit(&TCPServerLWIP, &TCPServerLWIPAdapter);
	TCPServerInit(&TCPServerLWIP, parent, &TCPServerInterface);
	
  return xResultAccept;
}
//==============================================================================
#endif //TCP_SERVER_LWIP_COMPONENT_ENABLE
