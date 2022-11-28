//==============================================================================
//module enable:

#include "TCPClient/Adapters/TCPClient_AdapterConfig.h"
#ifdef TCP_CLIENT_LWIP_ADAPTER_ENABLE
//==============================================================================
//includes:

#include "TCPClient_LWIP_Adapter.h"
#include "TCPClient_LWIP_TxAdapter.h"
#include "TCPClient_LWIP_RxAdapter.h"
#include "Common/xMemory.h"
#include <string.h>
//==============================================================================
//import:

//==============================================================================
//variables:


//==============================================================================
//functions:

static xResult private_TCP_CLIENT_open(TCPClientT* server);
//------------------------------------------------------------------------------

static void PrivateHandler(TCPClientT* server)
{
	TCPClientLWIPAdapterT* adapter = server->Adapter.Child;

	MX_LWIP_Process();

	xRxReceiverRead(&adapter->RxReceiver, &adapter->RxCircleBuffer);

	if (server->Status.State == TCPClientIsOpen && !adapter->Data.OtputUpdateTime)
	{
		adapter->Data.OtputUpdateTime = 20;

		tcp_output(adapter->Data.SocketHandle);
	}
}
//------------------------------------------------------------------------------

static void PrivateIRQListener(TCPClientT* server, void* reg)
{

}
//------------------------------------------------------------------------------

static void PrivateEventListener(TCPClientT* server, TCPClientAdapterEventSelector selector, void* args, ...)
{
	TCPClientLWIPAdapterT* adapter = server->Adapter.Child;

	switch ((int)selector)
	{
		case TCPClientAdapterEventUpdateTime:
			if (adapter->Data.OtputUpdateTime)
			{
				adapter->Data.OtputUpdateTime--;
			}
			break;

		default: return;
	}
}
//------------------------------------------------------------------------------

static xResult PrivateRequestListener(TCPClientT* server, TCPClientAdapterRequestSelector selector, void* arg, ...)
{
	switch ((int)selector)
	{
		default: return xResultRequestIsNotFound;
	}
	
	return xResultAccept;
}
//------------------------------------------------------------------------------
/**
  * @brief  This functions closes the tcp connection
  * @param  tcp_pcb: pointer on the tcp connection
  * @param  es: pointer on echo_state structure
  * @retval None
  */
static void private_tcp_client_close(TCPClientT* server)
{
	TCPClientLWIPAdapterT* adapter = server->Adapter.Child;
	server->Status.State = TCPClientClosed;

	if (!server)
	{
		return;
	}

	if (adapter->Data.SocketHandle)
	{
		/* remove all callbacks */
		/*
		tcp_arg(adapter->Data.Socket, NULL);
		tcp_sent(adapter->Data.Socket, NULL);
		tcp_recv(adapter->Data.Socket, NULL);
		tcp_err(adapter->Data.Socket, NULL);
		tcp_poll(adapter->Data.Socket, NULL, 0);
*/
		/* close tcp connection */
		tcp_close(adapter->Data.SocketHandle);
		//tcp_close(adapter->Data.Socket);

		//adapter->Data.Socket = NULL;
		//adapter->Data.SocketHandle = NULL;
	}
}
//------------------------------------------------------------------------------
/**
  * @brief  This function is the implementation for tcp_recv LwIP callback
  * @param  arg: pointer on a argument for the tcp_pcb connection
  * @param  tpcb: pointer on the tcp_pcb connection
  * @param  pbuf: pointer on the received pbuf
  * @param  err: error information regarding the reveived pbuf
  * @retval err_t: error code
  */
static err_t private_tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	TCPClientT* server = arg;
	TCPClientLWIPAdapterT* adapter = server->Adapter.Child;

	if (err != ERR_OK)
	{
		//free received pbuf
		if (p != NULL)
		{
			pbuf_free(p);
		}

		return err;
	}

	uint32_t len = 0;
	struct pbuf* element = p;

	while (len < p->tot_len && element)
	{
		//xRxReceiverReceive(&adapter->RxReceiver, element->payload, element->len);

		xCircleBufferAdd(&adapter->RxCircleBuffer, element->payload, element->len);

		len += element->len;
		element = element->next;
	}

	tcp_recved(tpcb, p->tot_len);
	pbuf_free(p);

	return ERR_OK;
}
//------------------------------------------------------------------------------
/**
  * @brief  This function implements the tcp_err callback function (called
  *         when a fatal tcp_connection error occurs.
  * @param  arg: pointer on argument parameter
  * @param  err: not used
  * @retval None
  */
static void private_tcp_client_error(void *arg, err_t err)
{
	TCPClientT* server = arg;

	server->Status.Error = TCPClientErrorAborted;

	private_TCP_CLIENT_close(server);
}
//------------------------------------------------------------------------------
/**
  * @brief  This function implements the tcp_poll LwIP callback function
  * @param  arg: pointer on argument passed to callback
  * @param  tpcb: pointer on the tcp_pcb for the current tcp connection
  * @retval err_t: error code
  */
static err_t private_tcp_client_poll(void *arg, struct tcp_pcb *tpcb)
{
	TCPClientT* server = arg;
	err_t ret_err;

	if (server != NULL)
	{
		ret_err = ERR_OK;
	}
	else
	{
		/* nothing to be done */
		tcp_abort(tpcb);
		ret_err = ERR_ABRT;
	}

	switch ((int)tpcb->state)
	{
		case CLOSE_WAIT:
			private_TCP_CLIENT_close(server);
			//private_TCP_CLIENT_open(server);
			break;
	}

	return ret_err;
}
//------------------------------------------------------------------------------
/**
  * @brief  This function is the implementation of tcp_accept LwIP callback
  * @param  arg: not used
  * @param  newpcb: pointer on tcp_pcb struct for the newly created tcp connection
  * @param  err: not used
  * @retval err_t: error status
  */
static err_t private_TCP_CLIENT_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	TCPClientT* server = arg;
	TCPClientLWIPAdapterT* adapter = server->Adapter.Child;

	err_t ret_err;

	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);

	/* set priority for the newly accepted tcp connection newpcb */
	tcp_setprio(newpcb, TCP_PRIO_MIN);

	if (arg != NULL)
	{
		server->Status.State = TCPClientIsOpen;
		adapter->Data.SocketHandle = newpcb;

		//pass newly allocated es structure as argument to newpcb
		tcp_arg(newpcb, arg);

		//initialize lwip tcp_recv callback function for newpcb
		tcp_recv(newpcb, private_tcp_client_recv);

		//initialize lwip tcp_err callback function for newpcb
		tcp_err(newpcb, private_tcp_client_error);

		//initialize lwip tcp_poll callback function for newpcb
		tcp_poll(newpcb, private_tcp_client_poll, 0);

		ret_err = ERR_OK;
	}
	else
	{
		//close tcp connection
		//tcp_echoserver_connection_close(newpcb, es);

		//return memory error/
		ret_err = ERR_MEM;
	}

  return ret_err;
}
//------------------------------------------------------------------------------

static xResult private_tcp_client_open(TCPClientT* server)
{
	TCPClientLWIPAdapterT* adapter = server->Adapter.Child;

	if (server->Status.State == TCPClientClosed)
	{
		if (!adapter->Data.Socket)
		{
			adapter->Data.Socket = tcp_new();
		}

		err_t err;

		//bind echo_pcb to port TCP_CLIENT_DEFAULT_PORT (ECHO protocol)
		err = tcp_bind(adapter->Data.Socket, IP_ADDR_ANY, TCP_CLIENT_DEFAULT_PORT);

		if (err == ERR_OK)
		{
			//tart tcp listening for echo_pcb
			adapter->Data.Socket = tcp_listen(adapter->Data.Socket);
			tcp_arg(adapter->Data.Socket, server);

			server->Status.State = TCPClientOpening;
			server->Status.Error = TCPClientErrorNo;

			//initialize LwIP tcp_accept callback function
			tcp_accept(adapter->Data.Socket, private_TCP_CLIENT_accept);

			return xResultAccept;
		}
		else
		{
			//deallocate the pcb
			memp_free(MEMP_TCP_PCB, adapter->Data.Socket);
		}
	}

	return xResultError;
}
//==============================================================================
//interfaces:

static TCPClientAdapterInterfaceT TCPClientAdapterInterface =
{
	INITIALIZATION_HANDLER(TCPClientAdapter, PrivateHandler),

	INITIALIZATION_IRQ_LISTENER(TCPClientAdapter, PrivateIRQListener),
	INITIALIZATION_EVENT_LISTENER(TCPClientAdapter, PrivateEventListener),
	INITIALIZATION_REQUEST_LISTENER(TCPClientAdapter, PrivateRequestListener),
};
//------------------------------------------------------------------------------
//initialization:

xResult TCPClientLWIPAdapterInit(TCPClientT* server, TCPClientLWIPAdapterT* adapter)
{
	if (!server || !adapter)
	{
		return xResultLinkError;
	}

	server->Adapter.Object.Description = "TCPClientLWIPAdapterT";
	server->Adapter.Object.Parent = server;

	server->Adapter.Interface = &TCPClientAdapterInterface;
	server->Adapter.Child = adapter;

	TCPClientLWIPRxAdapterInit(server, adapter);
	TCPClientLWIPTxAdapterInit(server, adapter);

	xRxTxBind(&server->Rx, &server->Tx);

	private_tcp_client_open(server);

  return xResultError;
}
//==============================================================================
#endif //TCP_CLIENT_LWIP_ADAPTER_ENABLE
