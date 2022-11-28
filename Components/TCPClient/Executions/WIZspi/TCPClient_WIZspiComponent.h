//==============================================================================
//module enable:

#include "TCPClient/TCPClient_ComponentConfig.h"
#ifdef TCP_CLIENT_WIZ_SPI_COMPONENT_ENABLE
//==============================================================================
//header:

#ifndef _TCP_CLIENT_WIZ_SPI_COMPONENT_H
#define _TCP_CLIENT_WIZ_SPI_COMPONENT_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "TCPServer_WIZspiComponentConfig.h"
#include "TCPServer/Controls/TCPServer.h"
//==============================================================================
//defines:


//==============================================================================
//functions:

xResult TCPServerWIZspiComponentInit(void* parent);

void _TCPServerWIZspiComponentHandler();
void _TCPServerWIZspiComponentTimeSynchronization();
void _TCPServerWIZspiComponentEventListener(TCPServerT* server, TCPServerEventSelector selector, void* arg, ...);
xResult _TCPServerWIZspiComponentRequestListener(TCPServerT* server, TCPServerRequestSelector selector, void* arg, ...);
void _TCPServerWIZspiComponentIRQListener(TCPServerT* port, ...);
//==============================================================================
//import:


//==============================================================================
//override:

#define TCPServerWIZspiComponentHandler() TCPServer(&TCPServerWIZspi)
#define TCPServerWIZspiComponentTimeSynchronization() TCPServer(&TCPServerWIZspi)

#define TCPServerWIZspiComponentIRQListener(server, ...) _TCPServerComponentIRQListener(server, ##__VA_ARGS__)

#define TCPServerWIZspiComponentEventListener(server, selector, arg, ...) _SerialPortUARTComponentEventListener(port, selector, arg, ##__VA_ARGS__)
#define TCPServerWIZspiComponentRequestListener(server, selector, arg, ...) _SerialPortUARTComponentRequestListener(port, selector, arg, ##__VA_ARGS__)
//==============================================================================
//export:

extern TCPServerT TCPServerWIZspi;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TCP_CLIENT_WIZ_SPI_COMPONENT_H
#endif //TCP_CLIENT_WIZ_SPI_COMPONENT_ENABLE
