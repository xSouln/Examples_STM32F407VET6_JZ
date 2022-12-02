//==============================================================================
#ifndef _ZIGBEE_H_
#define _ZIGBEE_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Zigbee/Controls/Zigbee_Types.h"
//==============================================================================
//macros:

#define ZigbeeEventTxIRQ(network) xTxIRQListener(&network.Tx)
#define ZigbeeEventRxIRQ(network) xTxIRQEventListener(&network.Rx)
//==============================================================================
//functions:

xResult ZigbeeInit(ZigbeeT* network, void* parent, ZigbeeInterfaceT* interface);

void _ZigbeeHandler(ZigbeeT* network);
void _ZigbeeTimeSynchronization(ZigbeeT* network);
void _ZigbeeIRQListener(ZigbeeT* Zigbee);
void _ZigbeeEventListener(ZigbeeT* network, ZigbeeEventSelector selector, void* arg, ...);
xResult _ZigbeeRequestListener(ZigbeeT* network, ZigbeeRequestSelector selector, void* arg, ...);
//==============================================================================
//imnetwork:


//==============================================================================
//override:

#define ZigbeeHandler(network) _ZigbeeHandler(network)
#define ZigbeeTimeSynchronization(network) _ZigbeeTimeSynchronization(network)

#define ZigbeeIRQListener(network) _ZigbeeIRQListener(network)

#define ZigbeeEventListener(network, selector, arg, ...) ((ZigbeeT*)network)->Interface->EventListener(network, selector, arg, ##__VA_ARGS__)
#define ZigbeeRequestListener(network, selector, arg, ...) ((ZigbeeT*)network)->Interface->RequestListener(network, selector, arg, ##__VA_ARGS__)
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_ZIGBEE_H_
