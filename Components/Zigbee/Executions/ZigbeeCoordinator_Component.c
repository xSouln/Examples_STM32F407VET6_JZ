//==============================================================================
//module enable:

#include "ZigbeeCoordinator_ComponentConfig.h"
#ifdef ZIGBEE_COORDINATOR_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "Zigbee_Component.h"

#ifdef TERMINAL_COMPONENT_ENABLE
#include "Terminal/Controls/Terminal.h"
#endif
//==============================================================================

static void _ZigbeeComponentEventListener(ZigbeeT* network, ZigbeeEventSelector selector, void* arg, ...)
{
	switch((int)selector)
	{
		case ZigbeeEventEndLine:
			#ifdef TERMINAL_COMPONENT_ENABLE
			TerminalReceiveData(&network->Rx,
								((ZigbeeReceivedDataT*)arg)->Data,
								((ZigbeeReceivedDataT*)arg)->Size);
			#endif
			break;

		case ZigbeeEventBufferIsFull:
			#ifdef TERMINAL_COMPONENT_ENABLE
			TerminalReceiveData(&network->Rx,
								((ZigbeeReceivedDataT*)arg)->Data,
								((ZigbeeReceivedDataT*)arg)->Size);
			#endif
			break;

		default: break;
	}
}
//------------------------------------------------------------------------------

static xResult _ZigbeeComponentRequestListener(ZigbeeT* network, ZigbeeRequestSelector selector, void* arg, ...)
{
	switch((int)selector)
	{
		default: return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------

void _ZigbeeComponentIRQListener(ZigbeeT* network)
{
	ZigbeeIRQListener(network);
}
//------------------------------------------------------------------------------
/**
 * @brief main handler
 */
void _ZigbeeComponentHandler(ZigbeeT* network)
{
	ZigbeeHandler(network);
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
void _ZigbeeComponentTimeSynchronization(ZigbeeT* network)
{
	ZigbeeTimeSynchronization(network);
}
//==============================================================================

static ZigbeeInterfaceT ZigbeeInterface =
{
	INITIALIZATION_EVENT_LISTENER(Zigbee, _ZigbeeComponentEventListener),
	INITIALIZATION_REQUEST_LISTENER(Zigbee, _ZigbeeComponentRequestListener)
};
//------------------------------------------------------------------------------
/**
 * @brief initializing the component
 * @param parent binding to the parent object
 * @return int
 */
xResult _ZigbeeComponentInit(ZigbeeT* network, void* parent)
{
	ZigbeeInit(network, parent, &ZigbeeInterface);

	return 0;
}
//==============================================================================
#endif //ZIGBEE_COMPONENT_ENABLE
