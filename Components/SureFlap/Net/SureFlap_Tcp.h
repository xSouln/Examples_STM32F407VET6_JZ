//==============================================================================
#ifndef _SUREFLAP_TCP_H
#define _SUREFLAP_TCP_H
//------------------------------------------------------------------------------
#include "SureFlap/SureFlap_ComponentConfig.h"
#ifdef SUREFLAP_COMPONENT_ENABLE
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "SureFlap_TcpTypes.h"
#include "SureFlap/Control/SureFlap_Types.h"
//==============================================================================
//defines:


//==============================================================================
//functions:

xResult _SureFlapTcpInit(SureFlapT* hub);
void _SureFlapTcpHandler(SureFlapTcpT* tcp);
void _SureFlapTcpTimeSynchronization(SureFlapTcpT* tcp);
xResult _SureFlapTcpAddToServerBuffer(SureFlapTcpT* tcp, uint64_t mac, void* data, uint16_t size);
//==============================================================================
//override:

#define SureFlapTcpInit(hub) _SureFlapTcpInit(hub)
#define SureFlapTcpHandler(tcp) _SureFlapTcpHandler(tcp)
#define SureFlapTcpTimeSynchronization(tcp) _SureFlapTcpTimeSynchronization(tcp)

#define SureFlapTcpAddToServerBuffer(tcp, mac, data, size) _SureFlapTcpAddToServerBuffer(tcp, mac, data, size)
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SUREFLAP_TCP_H
#endif //SUREFLAP_COMPONENT_ENABLE
