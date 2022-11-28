//==============================================================================
//includes:

#include "xRx.h"
//==============================================================================
//variables:

static const ObjectDescriptionT xRxObjectDescription =
{
	.Key = OBJECT_DESCRIPTION_KEY,
	.ObjectId = X_RX_OBJECT_ID,
	.Type = nameof(xRxT)
};
//==============================================================================
//functions:

//==============================================================================
//initialization:

xResult xRxInit(xRxT* rx,
								void* parent,
								xTxAdapterT* adapter,
								const xRxInterfaceT* interface)
{
	if (rx && interface && adapter)
	{
		rx->Object.Description = &xRxObjectDescription;
		rx->Object.Parent = parent;

		rx->Interface = interface;
		rx->Adapter = adapter;
		
		rx->Status.IsInit = true;
		
		return xResultAccept;
	}
	
	return xResultError;
}
//==============================================================================
