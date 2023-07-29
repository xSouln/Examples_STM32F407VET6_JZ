//==============================================================================
//includes:

#include "LWIP-Net-Adapter.h"
#include "Common/xMemory.h"
#include "Abstractions/xSystem/xSystem.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"
//==============================================================================
//defines:

#define SNTP_SERVER "pool.ntp.org"
#define SNTP_PORT 123
//==============================================================================
//variables:

static int keepAlive = 1;
static int keepIdle = 30;
static int keepInterval = 1;
static int keepCount = 2;
static xNetNTP_PacketT NTP_Packet;
static xNetAddressT ServerIpAddres;
//==============================================================================
//prototypes:


//==============================================================================
//functions:

static void PrivateSendEvent(xNetT* net, xNetEventSelector selector, void* arg)
{
	xNetEventListenerListElementT* element = xListStartEnumeration((xListT*)&net->EventListeners);

	while (element)
	{
		element->Value((void*)net, selector, arg);
		element = element->Next;
	}

	xListStopEnumeration((xListT*)&net->EventListeners);
}
//------------------------------------------------------------------------------
static void PrivateDHCP_Handler(xNetT* net)
{
	LWIP_NetAdapterT* adapter = (LWIP_NetAdapterT*)net->Adapter.Content;

	if (!net->PhyIsConnecnted || net->DHCP.State == xNetDHCP_StateIdle)
	{
		return;
	}

	switch ((int)net->DHCP.State)
	{
		case xNetDHCP_Starting:
		{
			ip4_addr_t startAddress = { 0 };

			dhcp_stop(adapter->netif);

			//reset address
			netif_set_addr(adapter->netif,
					(const ip4_addr_t*)&startAddress.addr,
					(const ip4_addr_t*)&startAddress.addr,
					(const ip4_addr_t*)&startAddress.addr);

			if (dhcp_start(adapter->netif) == ERR_OK)
			{
				net->DHCP.TimeStamp = xSystemGetTime(NULL);
				net->DHCP.State = xNetDHCP_Started;
				break;
			}

			net->DHCP.Result = xResultError;
			net->DHCP.State = xNetDHCP_StateIdle;
			PrivateSendEvent(net, xNetEventDHCP_StateChanged, 0);
			break;
		}
		case xNetDHCP_Started:
		{
			if (xSystemGetTime(NULL) - net->DHCP.TimeStamp > net->DHCP.TimeOut)
			{
				dhcp_stop(adapter->netif);
				net->DHCP.Result = xResultTimeOut;
				net->DHCP.State = xNetDHCP_StateIdle;
				PrivateSendEvent(net, xNetEventDHCP_StateChanged, 0);
				break;
			}

			if (adapter->netif->ip_addr.addr)
			{
				ServerIpAddres.Value = adapter->netif->ip_addr.addr;

				net->DHCP.Result = xResultAccept;
				net->DHCP.State = xNetDHCP_StateIdle;
				net->DHCP_Complite = true;
				PrivateSendEvent(net, xNetEventDHCP_StateChanged, 0);
			}

			break;
		}
	}
}
//------------------------------------------------------------------------------
static void PrivateSNTP_Handler(xNetT* net)
{
	if (!net->PhyIsConnecnted || net->SNTP.State == xNetSNTP_StateIdle || !net->DHCP_Complite)
	{
		return;
	}

	int sntpSocket = -1;

	switch(net->SNTP.State)
	{
		case xNetSNTP_Starting:
		{
			net->SNTP.State = xNetSNTP_Started;
			net->SNTP_Complite = false;
		}

		case xNetSNTP_Started:
		{
			ip_addr_t addr = { 0 };

			/* query host IP address */
			err_t err = netconn_gethostbyname(SNTP_SERVER, &addr);

			if(err != ERR_OK || addr.addr == 0)
			{
				goto error;
			}

			struct sockaddr_in hostAddress;
			hostAddress.sin_family = AF_INET;
			hostAddress.sin_port = 0;
			hostAddress.sin_addr.s_addr = IPADDR_ANY;

			struct sockaddr_in address;
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = addr.addr;
			address.sin_port = htons(SNTP_PORT);

			sntpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

			if (sntpSocket == -1)
			{
				goto error;
			}

			int32_t result = bind(sntpSocket, (struct sockaddr*)&hostAddress, sizeof(hostAddress));
			if (result != ERR_OK)
			{
				goto error;
			}

			struct timeval timeout = { 0 };
			timeout.tv_sec = 5000;

			setsockopt(sntpSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
			setsockopt(sntpSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

			memset(&NTP_Packet, 0, sizeof(NTP_Packet));
			NTP_Packet.flags.versionNumber = 3;
			NTP_Packet.flags.mode = 3;
			NTP_Packet.orig_ts_secs = xMemorySwap32(SNTP_EPOCH);

			result = sendto(sntpSocket, &NTP_Packet, sizeof(NTP_Packet), 0, (struct sockaddr*)&address, sizeof(address));
			if(result != sizeof(NTP_Packet))
			{
				goto error;
			}

			uint32_t packetLen = sizeof(address);
			result = recvfrom(sntpSocket, &NTP_Packet, sizeof(NTP_Packet), 0, (struct sockaddr*)&address, &packetLen);

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

			close(sntpSocket);

			net->SNTP.Result = xResultAccept;
			net->SNTP.State = xNetSNTP_StateIdle;
			net->SNTP_Complite = true;

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
	close(sntpSocket);
	net->SNTP.Result = xResultError;
	net->SNTP.State = xNetSNTP_StateIdle;
}
//------------------------------------------------------------------------------
static void PrivateHandler(xNetT* net)
{
	LWIP_NetAdapterT* adapter = (LWIP_NetAdapterT*)net->Adapter.Content;
	struct
	{
		uint32_t LastPhyIsConnecnted : 1;

	} Flags;

	Flags.LastPhyIsConnecnted = net->PhyIsConnecnted;
	net->PhyIsConnecnted = netif_is_link_up(adapter->netif);

	if (Flags.LastPhyIsConnecnted != net->PhyIsConnecnted)
	{
		if (!net->PhyIsConnecnted)
		{
			ip4_addr_t startAddress = { 0 };

			dhcp_stop(adapter->netif);

			//reset address
			netif_set_addr(adapter->netif,
					(const ip4_addr_t*)&startAddress.addr,
					(const ip4_addr_t*)&startAddress.addr,
					(const ip4_addr_t*)&startAddress.addr);
		}

		PrivateSendEvent(net, xNetEventPhy_ConnectionChanged, 0);
	}

	PrivateDHCP_Handler(net);
	PrivateSNTP_Handler(net);
}
//------------------------------------------------------------------------------
static void PrivateIRQ(xNetT* net, void* arg)
{
	
}
//------------------------------------------------------------------------------
static void PrivateCloseSocket(xNetSocketT* socket)
{
	if (socket && socket->State != xNetSocketIdle)
	{
		shutdown((int)socket->Handle, 0);
		close((int)socket->Handle);
		socket->Handle = (void*)-1;
		socket->State = xNetSocketIdle;
	}
}
//------------------------------------------------------------------------------
static void PrivateSocketHandler(xNetSocketT* socket)
{
	if ((int)socket->Handle != -1)
	{
		int optval;
    	socklen_t optlen = sizeof(optval);

		if (getsockopt((int)socket->Handle, SOL_SOCKET, SO_ERROR, &optval, &optlen) != 0 || optval != ERR_OK)
		{
			PrivateCloseSocket(socket);
		}
	}
}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(void* object, xNetAdapterRequestSelector selector, void* arg)
{
	switch ((int)selector)
	{
		case xNetAdapterInitTcpSocket:
		{
			xNetSocketT* sock = arg;

			if (sock->State != xNetSocketIdle)
			{
				return xResultError;
			}

			sock->Handle = (void*)socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

			if ((int)sock->Handle < 0)
			{
				return xResultError;
			}

			struct timeval timeout = { 0 };
			timeout.tv_sec = 3000;

			setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
			setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

			sock->Net = object;
			sock->State = xNetSocketInit;
		}
		break;

		case xNetAdapterClose:
		{
			PrivateCloseSocket((xNetSocketT*)object);
		}

		case xNetAdapterBind:
		{
			xNetSocketT* socket = object;

			struct sockaddr_storage dest_addr;
			struct sockaddr_in* dest_addr_ip4 = (struct sockaddr_in*)&dest_addr;
			dest_addr_ip4->sin_addr.s_addr = socket->Address.Value;
			dest_addr_ip4->sin_family = AF_INET;
			dest_addr_ip4->sin_port = htons(socket->Port);

			if (bind((int)socket->Handle, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) != 0)
			{
				PrivateCloseSocket(socket);
				return xResultError;
			}
		}
		break;

		case xNetAdapterListen:
		{
			xNetSocketT* socket = object;
			int max_count = *(int*)arg;

			setsockopt((int)socket->Handle, SOL_SOCKET, SO_REUSEADDR, &max_count, sizeof(max_count));

			struct sockaddr_storage dest_addr = { 0 };
			struct sockaddr_in* dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
			dest_addr_ip4->sin_addr.s_addr = socket->Address.Value;
			dest_addr_ip4->sin_family = AF_INET;
			dest_addr_ip4->sin_port = htons(socket->Port);

			if (bind((int)socket->Handle, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) != 0)
			{
				PrivateCloseSocket(socket);
				return xResultError;
			}

			if (listen((int)socket->Handle, max_count) != 0)
			{
				PrivateCloseSocket(socket);
				return xResultError;
			}

			//int keepAlive = 10000;
			//setsockopt((int)socket->Handle, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
			//setsockopt((int)socket->Handle, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
			//setsockopt((int)socket->Handle, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));

			socket->State = xNetSocketListen;
		}
		break;

		case xNetAdapterAccept:
		{
			xNetSocketT* server = object;
			xNetSocketT* client = arg;

			// Large enough for both IPv4 or IPv6
			struct sockaddr_storage source_addr;
        	socklen_t addr_len = sizeof(source_addr);

        	//fcntl((int)socket->Handle, F_SETFL, O_NONBLOCK);
			//int socket_number = accept((int)socket->Handle, (struct sockaddr*)&source_addr, &addr_len);
        	int socket_number = accept((int)server->Handle, (struct sockaddr*)&source_addr, &addr_len);

			if (socket_number < 0)
			{
				return xResultError;
			}

			client->Net = server->Net;
			client->Address.Value = ((struct sockaddr_in*)&source_addr)->sin_addr.s_addr;
			client->Handle = (void*)socket_number;

			setsockopt(socket_number, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
			setsockopt(socket_number, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
			setsockopt(socket_number, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
			setsockopt(socket_number, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

			client->State = xNetSocketEstablished;

			return xResultAccept;
		}

		case xNetAdapterGetPhyConnectionState:
		{
			xNetT* net = object;
			LWIP_NetAdapterT* adapter = (LWIP_NetAdapterT*)net->Adapter.Content;
			xNetAdapterGetPhyConnectionStateArg* request = arg;

			request->IsConnected = netif_is_up(adapter->netif);

			return xResultAccept;
		}

		case xNetAdapterDHCP_Start:
		{
			xNetT* net = object;
			xNetAdapterDHCP_StartArg* request = arg;

			if (net->DHCP.State == xNetDHCP_StateIdle)
			{
				net->DHCP.Result = xResulInProgress;
				net->DHCP.TimeOut = request->TimeOut;
				net->DHCP.State = xNetDHCP_Starting;

				break;
			}

			return xResultBusy;
		}

		case xNetAdapterGetHostByName:
		{
			xNetRequesGetHostByNameArgT* request = arg;
			ip_addr_t hostent_addr;

			err_t err = netconn_gethostbyname(request->Name, &hostent_addr);

			if (err != ERR_OK || hostent_addr.addr == 0)
			{
				return xResultError;
			}

			request->Result->Value = hostent_addr.addr;
		}
		break;

		case xNetAdapterConnect:
		{
			xNetSocketT* client = object;
			struct sockaddr_in server_addr;

			memset(&server_addr, 0, sizeof(server_addr));
			server_addr.sin_family = AF_INET;
			server_addr.sin_addr.s_addr = client->Address.Value;
			server_addr.sin_port = htons(client->Port);

			int result = connect((int)client->Handle, (struct sockaddr*)&server_addr, sizeof(server_addr));
			return result < 0 ? xResultError : xResultAccept;
		}
		break;

		case xNetAdapterInit:
		{

		}
		break;

		case xNetAdapterSNTP_Start:
		{
			xNetT* net = object;
			if (net->SNTP.State == xNetSNTP_StateIdle)
			{
				net->SNTP.State = xNetSNTP_Starting;
			}
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
	if ((int)socket->Handle != -1)
	{
		int sended = 0;
		uint8_t* mem = data;

		while (sended < size)
		{
			int len = send((int)socket->Handle, mem + sended, size - sended, 0);

			if(len < 0)
			{
				PrivateCloseSocket(socket);

				return -xResultError;
			}

			sended += len;
		}
		
		return sended;
	}

	return -xResultError;
}
//------------------------------------------------------------------------------
static int PrivateReceive(xNetSocketT* socket, void* data, int size)
{
	if ((int)socket->Handle != -1)
	{
		int received = recv((int)socket->Handle, data, size, 0);

		if (received < 0)
		{
			return -xResultError;
		}
		
		return received;
	}

	return -xResultError;
}
//==============================================================================
//initializations:

static xNetAdapterInterfaceT PrivateInterface =
{
	.Handler = (xNetAdapterHandlerT)PrivateHandler,
	.IRQ = (xNetAdapterIRQT)PrivateIRQ,
	.SocketHandler = (xNetAdapterSocketHandlerT)PrivateSocketHandler,

	.RequestListener = (xNetAdapterRequestListenerT)PrivateRequestListener,
	.EventListener = (xNetAdapterEventListenerT)PrivateEventListener,

	.Transmit = (xNetAdapterTransmitActionT)PrivateTransmit,
	.Receive = (xNetAdapterReceiveActionT)PrivateReceive,
};
//------------------------------------------------------------------------------
xResult LWIP_NetAdapterInit(xNetT* net, xNetAdapterInitT* init)
{
	if (!net || !init)
	{
		return xResultLinkError;
	}

	LWIP_NetAdapterInitT* adapterInit = init->Init;
	LWIP_NetAdapterT* adapter = init->Adapter;

	adapter->netif = adapterInit->gnetif;

	net->Adapter.Content = adapter;
	net->Adapter.Description = nameof(LWIP_NetAdapterT);

	net->Adapter.Interface = &PrivateInterface;
  
	return xResultError;
}
//==============================================================================
