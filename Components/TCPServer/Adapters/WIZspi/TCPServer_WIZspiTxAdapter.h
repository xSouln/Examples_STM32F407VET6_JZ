//==============================================================================
//module enable:

#include "TCPServer/Adapters/TCPServer_AdapterConfig.h"
#ifdef TCP_SERVER_WIZ_SPI_ADAPTER_ENABLE
//==============================================================================
//header:

#ifndef _TCP_SERVER_WIZ_SPI_TX_ADAPTER_H
#define _TCP_SERVER_WIZ_SPI_TX_ADAPTER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "TCPServer_WIZspiAdapter.h"
//==============================================================================
//functions:

xResult TCPServerWIZspiTxAdapterInit(TCPServerT* server, TCPServerWIZspiAdapterT* adapter);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TCP_SERVER_WIZ_SPI_TX_ADAPTER_H
#endif //TCP_SERVER_WIZ_SPI_ADAPTER_ENABLE
