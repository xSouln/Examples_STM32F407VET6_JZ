//==============================================================================
//includes:

#include "HostRequestControl-Component.h"
#include "Adapters/CAN_LocalRequestControl-Adapter.h"

#include "Components.h"
//==============================================================================
//defines:

#define HOST_REQUEST_BUFFER_SIZE 20
//==============================================================================
//import:


//==============================================================================
//variables:

static CAN_LocalRequestT CAN_LocalRequestBuffer[HOST_REQUEST_BUFFER_SIZE];

xRequestControlT HostRequestControl;
//==============================================================================
//functions:

void HostRequestControlComponentHandler()
{
	xRequestControlHandlerDirect(HostRequestControl);
}
//==============================================================================
//initializations:

CAN_LocalRequestControlAdapterT privateLocalRequestControlAdapter;
//==============================================================================
//initialization:

xResult HostRequestControlComponentInit(void* parent)
{
	CAN_LocalRequestControlAdapterInitT adapterInit;
	adapterInit.RequestBuffer = (void*)CAN_LocalRequestBuffer;
	adapterInit.RequestBufferSize = HOST_REQUEST_BUFFER_SIZE;
	adapterInit.Port = &CAN_LocalPort;
	CAN_LocalRequestControlAdapterInit(&HostRequestControl, &privateLocalRequestControlAdapter, &adapterInit);

	xRequestControlInitT controlInit;
	controlInit.Parent = parent;
	xRequestControlInit(&HostRequestControl, &controlInit);

	return xResultAccept;
}
//==============================================================================
