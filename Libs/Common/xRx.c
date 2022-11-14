//==============================================================================
#include "xRx.h"
//==============================================================================
inline void xRxHandler(xRxT* rx)
{
	rx->Interface->Handler(rx);
}
//------------------------------------------------------------------------------
inline void xRxEventListener(xRxT* rx, xRxEventSelector event, uint32_t args, uint32_t count)
{
	rx->Interface->EventListener(rx, event, args, count);
}
//==============================================================================
inline xResult xRxRequestListener(xRxT* rx, xRxRequestSelector selector, uint32_t args, uint32_t count)
{
	return rx->Interface->RequestListener(rx, selector, args, count);
}
//------------------------------------------------------------------------------
inline xResult xRxSetValue(xRxT* rx, xRxValueSelector selector, uint32_t value)
{
	return rx->Interface->SetValue(rx, selector, value);
}
//------------------------------------------------------------------------------
inline int xRxGetValue(xRxT* rx, xRxValueSelector selector)
{
	return rx->Interface->GetValue(rx, selector);
}
//------------------------------------------------------------------------------
inline int xRxReceive(xRxT* rx, uint8_t* data, uint32_t size)
{
	return rx->Interface->RequestListener(rx, xRxRequestReceive, (uint32_t)data, size);
}
//==============================================================================
xResult xRxPutInResponseBuffer(xRxT* rx, void* data, uint32_t size)
{
	return rx->Interface->RequestListener(rx, xRxRequestPutInResponseBuffer, (uint32_t)data, size);
}
//------------------------------------------------------------------------------
xResult xRxClearResponseBuffer(xRxT* rx)
{
	return rx->Interface->RequestListener(rx, xRxRequestClearBuffer, 0, 0);
}
//==============================================================================
xResult xRxInit(xRxT* rx,
								void* parent,
								xTxAdapterT* adapter,
								xRxInterfaceT* interface)
{
	if (rx && interface && adapter)
	{
		rx->Description = "xRxT";
		rx->Parent = parent;
		rx->Interface = interface;
		rx->Adapter = adapter;
		
		rx->Status.IsInit = true;
		
		return xResultAccept;
	}
	
	return xResultError;
}
//==============================================================================
