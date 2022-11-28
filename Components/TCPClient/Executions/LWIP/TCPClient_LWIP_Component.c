//==============================================================================
//module enable:

#include "TCPClient/TCPClient_ComponentConfig.h"
#ifdef TCP_CLIENT_LWIP_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "TCPClient_LWIP_Component.h"
#include "TCPClient/TCPClient_Component.h"
#include "TCPClient/Adapters/LWIP/TCPClient_LWIP_Adapter.h"

#include "Components.h"

#ifdef TERMINAL_COMPONENT_ENABLE
#include "Terminal/Controls/Terminal.h"
#endif
//==============================================================================
//defines:

#define RX_CIRCLE_BUF_SIZE_MASK (TCP_CLIENT_LWIP_RX_CIRCLE_BUF_SIZE_MASK)
#define RX_OBJECT_BUF_SIZE (TCP_CLIENT_LWIP_RX_OBJECT_BUF_SIZE)
//==============================================================================
//variables:

static uint8_t rx_circle_buf[RX_CIRCLE_BUF_SIZE_MASK + 1];
static uint8_t rx_object_buf[RX_OBJECT_BUF_SIZE];

TCPClientT TCPClientLWIP;
//==============================================================================
//functions:

static void _TCPClientLWIPComponentEventListener(TCPClientT* server, TCPClientEventSelector selector, void* arg, ...)
{
	switch ((uint8_t)selector)
	{
		case TCPClientEventEndLine:
			#ifdef TERMINAL_COMPONENT_ENABLE
			TerminalReceiveData(&server->Rx, ((TCPClientReceivedDataT*)arg)->Data, ((TCPClientReceivedDataT*)arg)->Size);
			#endif
			break;
		
		case TCPClientEventBufferIsFull:
			#ifdef TERMINAL_COMPONENT_ENABLE
			TerminalReceiveData(&server->Rx, ((TCPClientReceivedDataT*)arg)->Data, ((TCPClientReceivedDataT*)arg)->Size);
			#endif
			break;
	}
}
//------------------------------------------------------------------------------
static xResult _TCPClientLWIPComponentRequestListener(TCPClientT* server, TCPClientRequestSelector selector, void* arg, ...)
{
	switch ((uint8_t)selector)
	{
		case TCPClientRequestDelay:
			HAL_Delay((uint32_t)arg);
			break;
		
		default : return xResultRequestIsNotFound;
	}
	
	return xResultAccept;
}
//------------------------------------------------------------------------------

void _TCPClientLWIPComponentIRQListener()
{
	TCPClientIRQListener(&TCPClientLWIP);
}
//------------------------------------------------------------------------------
/**
 * @brief main handler
 */
void _TCPClientLWIPComponentHandler()
{
	TCPClientHandler(&TCPClientLWIP);
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
void _TCPClientLWIPComponentTimeSynchronization()
{
	TCPClientTimeSynchronization(&TCPClientLWIP);
}
//==============================================================================
//initializations:

static TCPClientInterfaceT TCPClientInterface =
{
	INITIALIZATION_EVENT_LISTENER(TCPClient, _TCPClientLWIPComponentEventListener),
	INITIALIZATION_REQUEST_LISTENER(TCPClient, _TCPClientLWIPComponentRequestListener)
};

//------------------------------------------------------------------------------

static TCPClientLWIPAdapterT TCPClientLWIPAdapter =
{
	.Handle = &heth
};
//==============================================================================
//initialization:

xResult _TCPClientLWIPComponentInit(void* parent)
{
	#ifdef TERMINAL_COMPONENT_ENABLE
	TCPClientLWIPAdapter.ResponseBuffer = &Terminal.ResponseBuffer;
	#endif
	
	xRxReceiverInit(&TCPClientLWIPAdapter.RxReceiver,
									&TCPClientLWIP.Rx,
									0,
									rx_object_buf,
									sizeof(rx_object_buf));

	xCircleBufferInit(&TCPClientLWIPAdapter.RxCircleBuffer,
						&TCPClientLWIP.Rx,
						rx_circle_buf,
						RX_CIRCLE_BUF_SIZE_MASK);

	TCPClientLWIPAdapterInit(&TCPClientLWIP, &TCPClientLWIPAdapter);
	TCPClientInit(&TCPClientLWIP, parent, &TCPClientInterface);
	
  return xResultAccept;
}
//==============================================================================
#endif //TCP_CLIENT_LWIP_COMPONENT_ENABLE
