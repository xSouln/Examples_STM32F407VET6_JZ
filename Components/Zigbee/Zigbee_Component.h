//==============================================================================
//module enable:

#include "Zigbee_ComponentConfig.h"
#ifdef ZIGBEE_COMPONENT_ENABLE
//==============================================================================
//header:

#ifndef _ZIGBEE_COMPONENT_H
#define _ZIGBEE_COMPONENT_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Zigbee_ComponentTypes.h"
#include "Zigbee/Zigbee_ComponentConfig.h"
#include "Zigbee/Controls/Zigbee.h"
//==============================================================================
//configurations:

#ifdef ZIGBEE_UART_COMPONENT_ENABLE
#include "Zigbee/Executions/Zigbee_UART_Component.h"
#endif
//==============================================================================
//defines:


//==============================================================================
//macros:


//==============================================================================
//functions:

xResult _ZigbeeComponentInit(ZigbeeT* network, void* parent);

void _ZigbeeComponentHandler(ZigbeeT* network);
void _ZigbeeComponentTimeSynchronization(ZigbeeT* network);

void _ZigbeeComponentIRQListener(ZigbeeT* network);
//==============================================================================
//exnetwork:


//==============================================================================
//override:

#ifndef ZigbeeComponentInit
#define ZigbeeComponentInit(network, parent)\
	_ZigbeeComponentInit(network, parent)
#endif
//------------------------------------------------------------------------------
#ifndef ZigbeeComponentHandler
#define ZigbeeComponentHandler(network)\
	_ZigbeeComponentHandler(network)
#endif
//------------------------------------------------------------------------------
#ifndef ZigbeeComponentTimeSynchronization
#define ZigbeeComponentTimeSynchronization(network)\
	_ZigbeeComponentTimeSynchronization(network)
#endif
//------------------------------------------------------------------------------
#ifndef ZigbeeComponentIRQListener
#define ZigbeeComponentIRQListener(network)\
	_ZigbeeComponentIRQListener(network)
#endif
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_ZIGBEE_COMPONENT_H
#endif //ZIGBEE_COMPONENT_ENABLE
