//==============================================================================
//module enable:

#include "Zigbee_ComponentConfig.h"
#ifdef ZIGBEE_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "Zigbee_Component.h"
#include "mac_api.h"
#include "common_sw_timer.h"

#ifdef TERMINAL_COMPONENT_ENABLE
#include "Terminal/Controls/Terminal.h"
#endif
//==============================================================================
//variables:

static uint8_t update_time;
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
	if (!update_time)
	{
		update_time = 5;

		wpan_task();
	}
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
void _ZigbeeComponentTimeSynchronization(ZigbeeT* network)
{
	if (update_time)
	{
		update_time--;
	}
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
	sw_timer_init();

	wpan_init();

	wpan_mlme_reset_req(true);

	return 0;
}
//==============================================================================
#endif //ZIGBEE_COMPONENT_ENABLE
