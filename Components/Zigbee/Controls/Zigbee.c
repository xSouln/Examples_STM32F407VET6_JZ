//==============================================================================
//includes:

#include "Common/xMemory.h"
#include "Zigbee.h"
#include "Zigbee/Zigbee_Component.h"
//==============================================================================
//variables:

//==============================================================================
//functions:

void _ZigbeeEventListener(ZigbeeT* network, ZigbeeEventSelector selector, void* arg, ...)
{
	switch((uint8_t)selector)
	{
		default: break;
	}
}
//------------------------------------------------------------------------------

xResult _ZigbeeRequestListener(ZigbeeT* network, ZigbeeRequestSelector selector, void* arg, ...)
{
	switch((uint8_t)selector)
	{
		default: return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------

void _ZigbeeIRQListener(ZigbeeT* network)
{

}
//------------------------------------------------------------------------------

void _ZigbeeHandler(ZigbeeT* network)
{
	network->Adapter.Interface->Handler(network);
}
//------------------------------------------------------------------------------

void _ZigbeeTimeSynchronization(ZigbeeT* network)
{

}
//==============================================================================
//initialization:

static const ObjectDescriptionT ZigbeeObjectDescription =
{
	.Key = OBJECT_DESCRIPTION_KEY,
	.ObjectId = ZIGBEE_OBJECT_ID,
	.Type = nameof(ZigbeeT)
};
//------------------------------------------------------------------------------

xResult ZigbeeInit(ZigbeeT* network, void* parent, ZigbeeInterfaceT* interface)
{
	if (network)
	{
		network->Object.Description = &ZigbeeObjectDescription;
		network->Object.Parent = parent;
		
		network->Interface = interface;
		
		network->Status.InitResult = xResultAccept;
		
		return xResultAccept;
	}
	
	return xResultError;
}
//==============================================================================
