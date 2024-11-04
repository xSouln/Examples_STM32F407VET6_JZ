//==============================================================================
//header:

#include "Net-Adapter.h"

#ifdef _NET_ADAPTER_H_
//==============================================================================
//includes:

#include "Common/xMemory.h"
#include "Abstractions/xSystem/xSystem.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
//==============================================================================
//defines:

#define SNTP_PORT 123
#define SOCKET_RX_BLOCK_TIME 500
#define SOCKET_TX_BLOCK_TIME 1000
//==============================================================================
//variables:

static xNetNTP_PacketT NTP_Packet;
//==============================================================================
//prototypes:


//==============================================================================
//functions:

static void privateSendEvent(xNetT* net, xNetEventSelector selector, void* arg)
{
	xNetEventSubscribersListElementT* element = xListStartEnumeration((xListT*)&net->Subscribers);

	while (element)
	{
		xNetEventSubscriberT* subscriber = element->Value;

		subscriber->EventListener(net, selector, subscriber, arg);

		element = element->Next;
	}

	xListStopEnumeration((xListT*)&net->Subscribers);
}
//------------------------------------------------------------------------------
/*
static void PrivateDHCP_Handler(xNetT* net)
{
	NetAdapterT* adapter = (NetAdapterT*)net->Adapter.Content;

	if (!net->PhyIsConnecnted || net->DHCP.State == xNetDHCP_StateIdle)
	{
		return;
	}


}
*/
//------------------------------------------------------------------------------
static void PrivateSNTP_Handler(xNetT* net)
{
	if (!net->PhyIsConnecnted || net->SNTP.State == xNetSNTP_StateIdle || !net->DHCP_Complite)
	{
		return;
	}

	xSocket_t socket = NULL;

	switch(net->SNTP.State)
	{
		case xNetSNTP_Starting:
		{
			net->SNTP.State = xNetSNTP_Started;
			net->SNTP_Complite = false;
		}

		case xNetSNTP_Started:
		{
			struct freertos_sockaddr sAddr = { 0 };

			socket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_DGRAM, FREERTOS_IPPROTO_UDP);

			if (socket == NULL)
			{
				goto error;
			}

			/*if (FreeRTOS_bind(socket, &sAddr, sizeof(sAddr)) != 0)
			{
				goto error;
			}*/

			uint32_t timeout = 5000;

			FreeRTOS_setsockopt(socket, 0, FREERTOS_SO_RCVTIMEO, &timeout, sizeof(timeout));
			FreeRTOS_setsockopt(socket, 0, FREERTOS_SO_SNDTIMEO, &timeout, sizeof(timeout));

			sAddr.sin_addr = FreeRTOS_gethostbyname(SNTP_SERVER);

			if (sAddr.sin_addr == 0)
			{
				goto error;
			}

			sAddr.sin_port = FreeRTOS_htons(SNTP_PORT);

			memset(&NTP_Packet, 0, sizeof(NTP_Packet));
			NTP_Packet.flags.versionNumber = 3;
			NTP_Packet.flags.mode = 3;
			NTP_Packet.orig_ts_secs = xMemorySwap32(SNTP_EPOCH);

			int result = FreeRTOS_sendto(socket, &NTP_Packet, sizeof(NTP_Packet), 0, &sAddr, sizeof(sAddr));
			if(result != sizeof(NTP_Packet))
			{
				goto error;
			}

			uint32_t packetLen = sizeof(sAddr);
			result = FreeRTOS_recvfrom(socket, &NTP_Packet, sizeof(NTP_Packet), 0, &sAddr, &packetLen);

			if(result != sizeof(NTP_Packet) || !NTP_Packet.stratum)
			{
				goto error;
			}

			NTP_Packet.tx_ts_secs = xMemorySwap32(NTP_Packet.tx_ts_secs) - SNTP_EPOCH;

			if(NTP_Packet.tx_ts_fraq & 0x80)
			{
				NTP_Packet.tx_ts_secs++;
			}

			net->SNTP.LastTime = NTP_Packet.tx_ts_secs;

			FreeRTOS_closesocket(socket);

			net->SNTP.Result = xResultAccept;
			net->SNTP.State = xNetSNTP_StateIdle;
			net->SNTP_Complite = true;
			privateSendEvent(net, xNetEventSNTP_Complite, 0);

			break;
		}

		default:
		{
			net->SNTP.State = xNetSNTP_StateIdle;
			return;
		}
	}

	return;

	error:;
	FreeRTOS_closesocket(socket);
	net->SNTP.Result = xResultError;
	net->SNTP.State = xNetSNTP_StateIdle;
	privateSendEvent(net, xNetEventSNTP_Error, 0);
}
//------------------------------------------------------------------------------
static void PrivateHandler(xNetT* net)
{
	net->PhyIsConnecnted = FreeRTOS_IsNetworkUp();

	uint8_t dhcpLastState = net->DHCP_Complite;

	net->DHCP_Complite = net->PhyIsConnecnted;

	if (dhcpLastState != net->DHCP_Complite && net->DHCP_Complite)
	{
		FreeRTOS_GetAddressConfiguration(&net->LocalAddress.Value,
				&net->NetMask.Value,
				&net->GatewayAddress.Value,
				&net->DNSServerAddress.Value);

		privateSendEvent(net, xNetEventDHCP_Complite, 0);
	}

	PrivateSNTP_Handler(net);
}
//------------------------------------------------------------------------------
static void PrivateCloseSocket(xNetSocketT* socket)
{
	if (socket && socket->State != xNetSocketIdle)
	{
		FreeRTOS_shutdown(socket->Handle, FREERTOS_SHUT_RDWR);
		FreeRTOS_closesocket(socket->Handle);

		socket->State = xNetSocketIdle;
		socket->Handle = NULL;
	}
}
//------------------------------------------------------------------------------
static void PrivateSocketHandler(xNetSocketT* socket)
{
	if ((int)socket->Handle != -1)
	{
		//int optval;
    	//socklen_t optlen = sizeof(optval);

    	//FreeRTOS_getsockopt(xSocket, 0, FREERTOS_SO_ERROR, &optval, &optlen);

		//if (FreeRTOS_getsockopt(xSocket, 0, FREERTOS_SO_ERROR, &optval, &optlen) != 0 || optval != ERR_OK)
		{
			//PrivateCloseSocket(socket);
		}
	}
}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(void* object, xNetAdapterRequestSelector selector, void* arg)
{
	#define CHECK_LWIP_SOCKET(socket) if (socket == 0) { return xResultError; }

	switch ((int)selector)
	{
		case xNetAdapterInitTcpSocket:
		{
			xNetSocketT* netSocket = arg;

			if (netSocket->State != xNetSocketIdle)
			{
				return xResultError;
			}

			Socket_t socket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);

			if (socket == FREERTOS_INVALID_SOCKET)
			{
				return xResultError;
			}

			uint32_t timeout = SOCKET_RX_BLOCK_TIME;
			FreeRTOS_setsockopt(netSocket->Handle, 0, FREERTOS_SO_RCVTIMEO, &timeout, sizeof(timeout));

			timeout = SOCKET_TX_BLOCK_TIME;
			FreeRTOS_setsockopt(netSocket->Handle, 0, FREERTOS_SO_SNDTIMEO, &timeout, sizeof(timeout));

			netSocket->Handle = socket;
			netSocket->Net = object;
			netSocket->State = xNetSocketInit;

			break;
		}

		case xNetAdapterClose:
		{
			PrivateCloseSocket((xNetSocketT*)object);
		}

		case xNetAdapterBind:
		{
			xNetSocketT* socket = object;

			CHECK_LWIP_SOCKET(socket->Handle);

			struct freertos_sockaddr serverAddress;
			serverAddress.sin_port = FreeRTOS_htons(socket->Port);
			serverAddress.sin_addr = socket->Address.Value;

			FreeRTOS_bind(socket->Handle, &serverAddress, sizeof(serverAddress));

			break;
		}

		case xNetAdapterListen:
		{
			xNetSocketT* socket = object;

			CHECK_LWIP_SOCKET(socket->Handle);

			struct freertos_sockaddr serverAddress;
			serverAddress.sin_port = FreeRTOS_htons(socket->Port);
			serverAddress.sin_addr = socket->Address.Value;

			if (FreeRTOS_bind(socket->Handle, &serverAddress, sizeof(serverAddress)) != 0)
			{
				PrivateCloseSocket(socket->Handle);
				return xResultError;
			}

			if (FreeRTOS_listen(socket->Handle, *(uint32_t*)arg) != 0)
			{
				PrivateCloseSocket(socket->Handle);
				return xResultError;
			}

			socket->State = xNetSocketListen;

			break;
		}

		case xNetAdapterAccept:
		{
			xNetSocketT* server = object;
			xNetSocketT* client = arg;

			CHECK_LWIP_SOCKET(server->Handle);

			struct freertos_sockaddr clientAddress;
			socklen_t clientAddressLength;

			Socket_t clientSocket = FreeRTOS_accept(server->Handle, &clientAddress, &clientAddressLength);

			if (clientSocket == 0)
			{
				return xResultError;
			}

			client->Net = server->Net;
			client->Address.Value = clientAddress.sin_addr;
			client->Handle = (void*)clientSocket;
/*
			setsockopt(socket_number, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
			setsockopt(socket_number, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
			setsockopt(socket_number, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
			setsockopt(socket_number, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));*/

			client->State = xNetSocketEstablished;

			return xResultAccept;
		}

		case xNetAdapterGetPhyConnectionState:
		{
			/*
			xNetT* net = object;
			NetAdapterT* adapter = (NetAdapterT*)net->Adapter.Content;
			xNetAdapterGetPhyConnectionStateArg* request = arg;
			*/

			return xResultAccept;
		}

		case xNetAdapterDHCP_Start:
		{
			/*
			xNetT* net = object;
			xNetAdapterDHCP_StartArg* request = arg;

			if (net->DHCP.State == xNetDHCP_StateIdle)
			{
				net->DHCP.Result = xResultInProgress;
				net->DHCP.TimeOut = request->TimeOut;
				net->DHCP.State = xNetDHCP_Starting;

				break;
			}
			*/

			return xResultBusy;
		}

		case xNetAdapterGetHostByName:
		{
			/*
			xNetRequesGetHostByNameArgT* request = arg;
			ip_addr_t hostent_addr;

			err_t err = netconn_gethostbyname(request->Name, &hostent_addr);

			if (err != ERR_OK || hostent_addr.addr == 0)
			{
				return xResultError;
			}

			request->Result->Value = hostent_addr.addr;
			*/

			break;
		}

		case xNetAdapterConnect:
		{
			/*
			xNetSocketT* client = object;

			CHECK_LWIP_SOCKET(client->Handle);

			struct sockaddr_in server_addr;

			memset(&server_addr, 0, sizeof(server_addr));
			server_addr.sin_family = AF_INET;
			server_addr.sin_addr.s_addr = client->Address.Value;
			server_addr.sin_port = htons(client->Port);

			int result = connect((int)client->Handle, (struct sockaddr*)&server_addr, sizeof(server_addr));
			return result < 0 ? xResultError : xResultAccept;
			*/

			break;
		}

		case xNetAdapterInit:
		{

		}
		break;

		case xNetAdapterSNTP_Start:
		{
			xNetT* net = object;

			if (net->SNTP.State != xNetSNTP_StateIdle)
			{
				return xResultBusy;
			}

			net->SNTP.State = xNetSNTP_Starting;
			break;
		}

		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static void PrivateEventListener(void* object, xNetAdapterEventSelector selector, void* arg)
{
	switch((int)selector)
	{
		default: return;
	}
}
//------------------------------------------------------------------------------
static int PrivateTransmit(xNetSocketT* socket, void* data, int size)
{
	if (socket->Handle == FREERTOS_INVALID_SOCKET || socket->Handle == NULL)
	{
		return -xResultError;
	}

	int sended = 0;
	uint8_t* mem = data;

	while (sended < size)
	{
		int len = FreeRTOS_send(socket->Handle, mem + sended, size - sended, FREERTOS_MSG_DONTWAIT);

		if(len < 0)
		{
			PrivateCloseSocket(socket);

			return -xResultError;
		}

		sended += len;
	}

	return sended;
}
//------------------------------------------------------------------------------
static int PrivateReceive(xNetSocketT* socket, void* buffer, int size)
{
	if (socket->Handle == FREERTOS_INVALID_SOCKET || socket->Handle == NULL)
	{
		return -xResultError;
	}

	BaseType_t bytesRead = FreeRTOS_recv(socket->Handle, buffer, size, FREERTOS_MSG_DONTWAIT);

	if (bytesRead < 0)
	{
		PrivateCloseSocket(socket);
		
		return -xResultError;
	}

	return bytesRead;
}
//==============================================================================
//initializations:

static xNetAdapterInterfaceT privateInterface =
{
	.Handler = (xNetAdapterHandlerT)PrivateHandler,
	.SocketHandler = (xNetAdapterSocketHandlerT)PrivateSocketHandler,

	.RequestListener = (xNetAdapterRequestListenerT)PrivateRequestListener,
	.EventListener = (xNetAdapterEventListenerT)PrivateEventListener,

	.Transmit = (xNetAdapterTransmitActionT)PrivateTransmit,
	.Receive = (xNetAdapterReceiveActionT)PrivateReceive,
};
//------------------------------------------------------------------------------
xResult NetAdapterInit(xNetT* net, NetAdapterT* adapter, NetAdapterInitT* adapterInit)
{
	if (!net || !adapter)
	{
		return xResultLinkError;
	}

	net->Adapter.Content = adapter;
	net->Adapter.Description = nameof(NetAdapterT);

	net->Adapter.Interface = &privateInterface;
  
	return xResultError;
}
//==============================================================================
#endif //_NET_ADAPTER_H_
