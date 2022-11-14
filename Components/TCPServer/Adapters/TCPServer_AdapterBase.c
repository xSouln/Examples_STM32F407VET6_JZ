//==============================================================================
#include "TCPServer_AdapterBase.h"
#include "TCPServer/Controls/TCPServer_Types.h"
//==============================================================================
static void Handler(TCPServerT* server)
{
	
}
//------------------------------------------------------------------------------
static void EventListener(TCPServerT* server, TCPServerAdapterEventSelector event, uint32_t args, uint32_t count)
{
	switch ((int)event)
	{
		default : break;
	}
}
//------------------------------------------------------------------------------
static xResult RequestListener(TCPServerT* server, TCPServerAdapterRequestSelector event, uint32_t args, uint32_t count)
{
	switch ((int)event)
	{
		default : return xResultRequestIsNotFound;
	}
	
	return xResultAccept;
}
//------------------------------------------------------------------------------
static uint32_t GetValue(TCPServerT* server, TCPServerAdapterValueSelector event, uint32_t args, uint32_t count)
{
	switch ((int)event)
	{
		default : break;
	}
	
	return 0;
}
//------------------------------------------------------------------------------
static xResult SetValue(TCPServerT* server, TCPServerAdapterValueSelector event, uint32_t args, uint32_t count)
{
	switch ((int)event)
	{
		default : return xResultValueIsNotFound;
	}
	
	return xResultAccept;
}
//==============================================================================
static TCPServerAdapterInterfaceT Interface =
{
	.Handler = (TCPServerAdapterHandlerT)Handler,
	.EventListener = (TCPServerAdapterEventListenerT)EventListener,
	.RequestListener = (TCPServerAdapterRequestListenerT)RequestListener,
	.GetValue = (TCPServerAdapterActionGetValueT)GetValue,
	.SetValue = (TCPServerAdapterActionSetValueT)SetValue,
};
//------------------------------------------------------------------------------
xResult TCPServerAdapterBaseInit(TCPServerAdapterBaseT* adapter)
{
	if (adapter)
	{
		adapter->Interface = &Interface;
	}
	return xResultAccept;
}
//==============================================================================
