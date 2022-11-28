//==============================================================================
//header:

#ifndef TCP_SERVER_H
#define TCP_SERVER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "TCPServer/Controls/TCPServer_Types.h"
//==============================================================================
//functions:

xResult TCPServerInit(TCPServerT* server, void* parent, TCPServerInterfaceT* interface);

void _TCPServerHandler(TCPServerT* server);
void _TCPServerTimeSynchronization(TCPServerT* server);
void _TCPServerIRQListener(TCPServerT* server);

void _TCPServerHandlerEventListener(TCPServerT* server, TCPServerEventSelector selector, void* arg, ...);
void _TCPServerHandlerRequestListener(TCPServerT* server, TCPServerRequestSelector selector, void* arg, ...);
//==============================================================================
//import:


//==============================================================================
//override:

#define TCPServerHandler(server) _TCPServerHandler(server)
#define TCPServerTimeSynchronization(server) _TCPServerTimeSynchronization(server)
#define TCPServerIRQListener(server) //_TCPServerIRQtListener(server)

#define TCPServerEventListener(server, selector, arg, ...) ((TCPServerT*)server)->Interface->EventListener(server, selector, arg, ##__VA_ARGS__)
#define TCPServerRequestListener(server, selector, arg, ...) ((TCPServerT*)server)->Interface->RequestListener(server, selector, arg, ##__VA_ARGS__)

//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //TCP_SERVER_H
