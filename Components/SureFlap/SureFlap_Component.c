//==============================================================================
//module enable:

#include "SureFlap_ComponentConfig.h"
#ifdef SUREFLAP_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "SureFlap_Component.h"
#include "SureFlap/Adapters/Zigbee_ASF/Zigbee_ASF_Adapter.h"

#ifdef TERMINAL_COMPONENT_ENABLE
#include "Terminal/Controls/Terminal.h"
#endif
//==============================================================================
//variables:

SureFlapT SureFlap;
//==============================================================================

static void _SureFlapComponentEventListener(SureFlapT* network, SureFlapEventSelector selector, void* arg, ...)
{
	switch((int)selector)
	{
		default: break;
	}
}
//------------------------------------------------------------------------------

static xResult _SureFlapComponentRequestListener(SureFlapT* network, SureFlapRequestSelector selector, void* arg, ...)
{
	switch((int)selector)
	{
		default: return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
/**
 * @brief main handler
 */
void _SureFlapComponentHandler()
{
	SureFlapZigbeeHandler(&SureFlap.Zigbee);
	SureFlapHandler(&SureFlap);
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
void _SureFlapComponentTimeSynchronization()
{
	SureFlapZigbeeTimeSynchronization(&SureFlap.Zigbee);
	SureFlapTimeSynchronization(&SureFlap);
}
//==============================================================================

static SureFlapInterfaceT SureFlapInterface =
{
	INITIALIZATION_EVENT_LISTENER(SureFlap, _SureFlapComponentEventListener),
	INITIALIZATION_REQUEST_LISTENER(SureFlap, _SureFlapComponentRequestListener)
};

static SureFlapZigbee_ASF_AdapterT SureFlapZigbee_ASF_Adapter =
{
	0
};
//------------------------------------------------------------------------------
/**
 * @brief initializing the component
 * @param parent binding to the parent object
 * @return int
 */
xResult _SureFlapComponentInit(void* parent)
{
	SureFlapZigbee_ASF_AdapterInit(&SureFlap.Zigbee, &SureFlapZigbee_ASF_Adapter);

	SureFlapInit(&SureFlap, parent, &SureFlapInterface);

	SureFlapZigbeeStartNetwork(&SureFlap.Zigbee);

	return 0;
}
//==============================================================================
#endif //SUREFLAP_COMPONENT_ENABLE
