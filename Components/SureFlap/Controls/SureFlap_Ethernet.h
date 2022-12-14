//==============================================================================
#ifndef _SUREFLAP_ETHERNET_H
#define _SUREFLAP_ETHERNET_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "SureFlap_EthernetTypes.h"
#include "SureFlap_Types.h"
//==============================================================================
//defines:


//==============================================================================
//functions:

xResult _SureFlapEthernetInit(SureFlapT* hub);
void _SureFlapEthernetHandler(SureFlapEthernetT* network);
void _SureFlapEthernetTimeSynchronization(SureFlapEthernetT* network);
//==============================================================================
//override:

#define SureFlapEthernetInit(hub) _SureFlapEthernetInit(hub)
#define SureFlapEthernetHandler(network) _SureFlapEthernetHandler(network)
#define SureFlapEthernetTimeSynchronization(network) _SureFlapEthernetTimeSynchronization(network)
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SUREFLAP_ETHERNET_H
