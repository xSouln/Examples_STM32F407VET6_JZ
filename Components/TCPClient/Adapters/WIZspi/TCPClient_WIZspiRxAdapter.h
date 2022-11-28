//==============================================================================
//module enable:

#include "TCPClient/Adapters/TCPClient_AdapterConfig.h"
#ifdef TCP_CLIENT_WIZ_SPI_ADAPTER_ENABLE
//==============================================================================
//header:

#ifndef _TCP_CLIENT_WIZ_SPI_RX_ADAPTER_H
#define _TCP_CLIENT_WIZ_SPI_RX_ADAPTER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "TCPClient_WIZspiAdapter.h"
//==============================================================================
//functions:

xResult TCPClientWIZspiRxAdapterInit(TCPClientT* server, TCPClientWIZspiAdapterT* adapter);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TCP_CLIENT_WIZ_SPI_RX_ADAPTER_H
#endif //TCP_CLIENT_WIZ_SPI_ADAPTER_ENABLE
