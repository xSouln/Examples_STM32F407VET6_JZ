//==============================================================================
//header:

#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "TCPClient/Controls/TCPClient_Types.h"
//==============================================================================
//functions:

xResult TCPClientInit(TCPClientT* server, void* parent, TCPClientInterfaceT* interface);

void _TCPClientHandler(TCPClientT* server);
void _TCPClientTimeSynchronization(TCPClientT* server);
void _TCPClientIRQListener(TCPClientT* server);

void _TCPClientHandlerEventListener(TCPClientT* server, TCPClientEventSelector selector, void* arg, ...);
void _TCPClientHandlerRequestListener(TCPClientT* server, TCPClientRequestSelector selector, void* arg, ...);
//==============================================================================
//import:


//==============================================================================
//override:

#define TCPClientHandler(server) _TCPClientHandler(server)
#define TCPClientTimeSynchronization(server) _TCPClientTimeSynchronization(server)
#define TCPClientIRQListener(server) //_TCPClientIRQtListener(server)

#define TCPClientEventListener(server, selector, arg, ...) ((TCPClientT*)server)->Interface->EventListener(server, selector, arg, ##__VA_ARGS__)
#define TCPClientRequestListener(server, selector, arg, ...) ((TCPClientT*)server)->Interface->RequestListener(server, selector, arg, ##__VA_ARGS__)

//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //TCP_CLIENT_H
