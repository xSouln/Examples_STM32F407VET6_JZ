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

void TCPServerHandler(TCPServerT* server);

xResult TCPServerInit(TCPServerT* server, void* parent, TCPServerInterfaceT* interface);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //TCP_SERVER_H
