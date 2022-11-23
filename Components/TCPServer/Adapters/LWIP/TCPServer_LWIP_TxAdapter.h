//==============================================================================
//module enable:

#include "TCPServer/Adapters/TCPServer_AdapterConfig.h"
#ifdef TCP_SERVER_LWIP_ADAPTER_ENABLE
//==============================================================================
//header:

#ifndef _TCP_SERVER_LWIP_TX_ADAPTER_H
#define _TCP_SERVER_LWIP_TX_ADAPTER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "TCPServer_LWIP_Adapter.h"
//==============================================================================
//functions:

xResult TCPServerLWIPTxAdapterInit(TCPServerT* server, TCPServerLWIPAdapterT* adapter);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TCP_SERVER_LWIP_TX_ADAPTER_H
#endif //TCP_SERVER_LWIP_ADAPTER_ENABLE
