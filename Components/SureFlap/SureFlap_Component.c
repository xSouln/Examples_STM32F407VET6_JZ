//==============================================================================
//module enable:

#include "SureFlap_ComponentConfig.h"
#ifdef SUREFLAP_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "SureFlap_Component.h"
#include "Adapters/ASF/Zigbee_ASF_Adapter.h"

#ifdef TERMINAL_COMPONENT_ENABLE
#include "Terminal/Controls/Terminal.h"
#endif
//==============================================================================
//variables:

SureFlapT SureFlap;
//==============================================================================
/**
 * @brief main handler
 */
void _SureFlapComponentHandler()
{
	SureFlapZigbeeHandler(&SureFlap.Zigbee);
	SureFlapDeviceHandler(&SureFlap.DeviceControl);
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


//==============================================================================

static SureFlapInterfaceT SureFlapInterface =
{

};
//------------------------------------------------------------------------------

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
