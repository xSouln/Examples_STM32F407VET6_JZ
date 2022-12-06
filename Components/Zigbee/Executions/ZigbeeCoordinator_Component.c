//==============================================================================
//module enable:

#include "ZigbeeCoordinator_ComponentConfig.h"
#ifdef ZIGBEE_COORDINATOR_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "ZigbeeCoordinator_Component.h"
#include "Zigbee/Adapters/ASF/Zigbee_ASF_Adapter.h"

#ifdef TERMINAL_COMPONENT_ENABLE
#include "Terminal/Controls/Terminal.h"
#endif
//==============================================================================
//variables:

ZigbeeT ZigbeeCoordinator;
//==============================================================================

static void _ZigbeeCoordinatorComponentEventListener(ZigbeeT* network, ZigbeeEventSelector selector, void* arg, ...)
{
	switch((int)selector)
	{
		default: break;
	}
}
//------------------------------------------------------------------------------

static xResult _ZigbeeCoordinatorComponentRequestListener(ZigbeeT* network, ZigbeeRequestSelector selector, void* arg, ...)
{
	switch((int)selector)
	{
		default: return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------

void _ZigbeeCoordinatorComponentIRQListener()
{
	ZigbeeIRQListener(&ZigbeeCoordinator);
}
//------------------------------------------------------------------------------
/**
 * @brief main handler
 */
void _ZigbeeCoordinatorComponentHandler()
{
	ZigbeeHandler(&ZigbeeCoordinator);
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
void _ZigbeeCoordinatorComponentTimeSynchronization()
{
	ZigbeeTimeSynchronization(&ZigbeeCoordinator);
}
//==============================================================================
//initialization:

static ZigbeeInterfaceT PrivateZigbeeCoordinatorInterface =
{
	INITIALIZATION_EVENT_LISTENER(Zigbee, _ZigbeeCoordinatorComponentEventListener),
	INITIALIZATION_REQUEST_LISTENER(Zigbee, _ZigbeeCoordinatorComponentRequestListener)
};
//------------------------------------------------------------------------------

static Zigbee_ASF_AdapterT PrivateZigbee_ASF_Adapter =
{
	0
};
//==============================================================================
/**
 * @brief initializing the component
 * @param parent binding to the parent object
 * @return int
 */
xResult _ZigbeeCoordinatorComponentInit(void* parent)
{
	ZigbeeInit(&ZigbeeCoordinator, parent, &PrivateZigbeeCoordinatorInterface);
	Zigbee_ASF_AdapterInit(&ZigbeeCoordinator, &PrivateZigbee_ASF_Adapter);

	return 0;
}
//==============================================================================
#endif //ZIGBEE_COMPONENT_ENABLE
