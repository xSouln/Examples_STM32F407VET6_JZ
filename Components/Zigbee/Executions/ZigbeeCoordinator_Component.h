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

#include "Zigbee/Controls/Zigbee.h"
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

xResult _ZigbeeCoordinatorComponentInit(void* parent);

void _ZigbeeCoordinatorComponentHandler();
void _ZigbeeCoordinatorComponentTimeSynchronization();
//==============================================================================
//macros:

#define ZigbeeCoordinatorComponentInit(parent) _ZigbeeCoordinatorComponentInit(parent)
#define ZigbeeCoordinatorComponentHandler() ZigbeeHandler(&ZigbeeCoordinator)
#define ZigbeeCoordinatorComponentTimeSynchronization(parent) ZigbeeTimeSynchronization(&ZigbeeCoordinator)
//==============================================================================
//override:

#define ZigbeeComponentInit(parent) ZigbeeCoordinatorComponentInit(parent)
#define ZigbeeComponentHandler() ZigbeeCoordinatorComponentHandler()
#define ZigbeeComponentTimeSynchronization() ZigbeeCoordinatorComponentTimeSynchronization()
//==============================================================================
//export:

extern ZigbeeT ZigbeeCoordinator;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_ZIGBEE_COORDINATOR_COMPONENT_H
#endif //ZIGBEE_COORDINATOR_COMPONENT_ENABLE
