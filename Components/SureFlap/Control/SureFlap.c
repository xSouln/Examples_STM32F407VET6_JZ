//==============================================================================
#include "SureFlap_Config.h"
#ifdef SUREFLAP_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "SureFlap.h"
//==============================================================================
//variables:

//==============================================================================
//functions:

void _SureFlapEventListener(SureFlapT* hub, SureFlapEventSelector selector, void* arg, ...)
{
	switch((uint8_t)selector)
	{
		default: break;
	}
}
//------------------------------------------------------------------------------

xResult _SureFlapRequestListener(SureFlapT* hub, SureFlapRequestSelector selector, void* arg, ...)
{
	switch((uint8_t)selector)
	{
		default: return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------

void _SureFlapIRQListener(SureFlapT* hub)
{

}
//------------------------------------------------------------------------------

void _SureFlapHandler(SureFlapT* hub)
{
	extern void SureFlapDeviceHubHandler(SureFlapDeviceControlT* control);
	//network->Adapter.Interface->Handler(network);
	//SureFlapZigbeeHandler(&hub->Zigbee);

	SureFlapDeviceHubHandler(&hub->DeviceControl);
}
//------------------------------------------------------------------------------

void _SureFlapTimeSynchronization(SureFlapT* hub)
{
	//network->Adapter.Interface->TimeSynchronization(hub);
	//SureFlapZigbeeTimeSynchronization(&hub->Zigbee);
}
//------------------------------------------------------------------------------

uint32_t _SureFlapGetTime(SureFlapT* hub)
{
	return 0;
}
//==============================================================================
//initialization:

static const ObjectDescriptionT SureFlapObjectDescription =
{
	.Key = OBJECT_DESCRIPTION_KEY,
	.ObjectId = SUREFLAPE_OBJECT_ID,
	.Type = nameof(SureFlapT)
};
//------------------------------------------------------------------------------

xResult SureFlapInit(SureFlapT* hub, void* parent, SureFlapInterfaceT* interface)
{
	if (hub)
	{
		hub->Object.Description = &SureFlapObjectDescription;
		hub->Object.Parent = parent;
		
		hub->Interface = interface;
		
		SureFlapDeviceInit(hub);
		SureFlapZigbeeInit(hub);

		return xResultAccept;
	}
	
	return xResultError;
}
//==============================================================================
#endif //SUREFLAP_COMPONENT_ENABLE

