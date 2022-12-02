//==============================================================================
//module enable:

#include "ZigbeeCoordinator_ComponentConfig.h"
#ifdef ZIGBEE_COORDINATOR_COMPONENT_ENABLE
//==============================================================================
//header:

#ifndef _ZIGBEE_COORDINATOR_COMPONENT_H
#define _ZIGBEE_COORDINATOR_COMPONENT_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "ZigbeeCoordinator/ZigbeeCoordinator_ComponentTypes.h"
#include "ZigbeeCoordinator/ZigbeeCoordinator_ComponentConfig.h"
#include "ZigbeeCoordinator/Controls/ZigbeeCoordinator.h"
//==============================================================================
//configurations:

#ifdef ZigbeeCoordinator_UART_COMPONENT_ENABLE
#include "ZigbeeCoordinator/Executions/ZigbeeCoordinator_UART_Component.h"
#endif
//==============================================================================
//defines:


//==============================================================================
//macros:


//==============================================================================
//functions:

xResult _ZigbeeCoordinatorComponentInit(ZigbeeCoordinatorT* network, void* parent);

void _ZigbeeCoordinatorComponentHandler(ZigbeeCoordinatorT* network);
void _ZigbeeCoordinatorComponentTimeSynchronization(ZigbeeCoordinatorT* network);

void _ZigbeeCoordinatorComponentIRQListener(ZigbeeCoordinatorT* network);
//==============================================================================
//exnetwork:


//==============================================================================
//override:

#ifndef ZigbeeCoordinatorComponentInit
#define ZigbeeCoordinatorComponentInit(network, parent)\
	_ZigbeeCoordinatorComponentInit(network, parent)
#endif
//------------------------------------------------------------------------------
#ifndef ZigbeeCoordinatorComponentHandler
#define ZigbeeCoordinatorComponentHandler(network)\
	_ZigbeeCoordinatorComponentHandler(network)
#endif
//------------------------------------------------------------------------------
#ifndef ZigbeeCoordinatorComponentTimeSynchronization
#define ZigbeeCoordinatorComponentTimeSynchronization(network)\
	_ZigbeeCoordinatorComponentTimeSynchronization(network)
#endif
//------------------------------------------------------------------------------
#ifndef ZigbeeCoordinatorComponentIRQListener
#define ZigbeeCoordinatorComponentIRQListener(network)\
	_ZigbeeCoordinatorComponentIRQListener(network)
#endif
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_ZIGBEE_COORDINATOR_COMPONENT_H
#endif //ZIGBEE_COORDINATOR_COMPONENT_ENABLE
