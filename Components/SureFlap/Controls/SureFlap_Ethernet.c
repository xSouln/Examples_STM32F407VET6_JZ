//==============================================================================
//includes:

#include "SureFlap_Ethernet.h"
//==============================================================================
//variables:

//==============================================================================
//functions:

//------------------------------------------------------------------------------

void _SureFlapEthernetHandler(SureFlapEthernetT* network)
{

}
//------------------------------------------------------------------------------

void _SureFlapEthernetTimeSynchronization(SureFlapEthernetT* network)
{

}
//==============================================================================
//initialization:

xResult _SureFlapEthernetInit(SureFlapT* hub)
{
	if (hub)
	{
		hub->Zigbee.Object.Description = nameof(SureFlapEthernetT);
		hub->Zigbee.Object.Parent = hub;

		return xResultAccept;
	}
	
	return xResultError;
}
//==============================================================================
