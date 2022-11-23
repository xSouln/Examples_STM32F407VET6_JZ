//==============================================================================
#include "WS2812_DrawManagerBase.h"
//==============================================================================
static void PrivateDrawManageHandler(WS2812_DrawManagerBaseT* manager)
{

}
//------------------------------------------------------------------------------
static xResult PrivateDrawManageRequestListener(WS2812_DrawManagerBaseT* manager, WS2812_DrawManagerRequestSelector selector, uint32_t args, uint32_t count)
{
	switch ((int)selector)
	{
		default : return xResultRequestIsNotFound;
	}
	
	return xResultAccept;
}
//------------------------------------------------------------------------------
static void PrivateDrawManageEventListener(WS2812_DrawManagerBaseT* manager, WS2812_DrawManagerEventSelector selector, uint32_t args, uint32_t count)
{
	switch ((int)selector)
	{
		default: break;
	}
}
//------------------------------------------------------------------------------
static uint32_t PrivateDrawManageGetValue(WS2812_DrawManagerBaseT* manager, WS2812_DrawManagerValueSelector selector)
{
	switch ((int)selector)
	{
		default : break;
	}
	
	return 0;
}
//------------------------------------------------------------------------------
static xResult PrivateDrawManageSetValue(WS2812_DrawManagerBaseT* manager, WS2812_DrawManagerValueSelector selector, uint32_t value)
{
	switch ((int)selector)
	{
		default : return xResultValueIsNotFound;
	}
	
	return xResultAccept;
}
//==============================================================================
xResult WS2812_DrawManageSetColor(WS2812_DrawManagerBaseT* manager, WS2812_ColorT color)
{
	return manager->SetValue(manager, WS2812_DrawManagerValueColor, (uint32_t)&color);
}
//==============================================================================
xResult WS2812_DrawManagerBaseInit(WS2812_DrawManagerBaseT* manager, void* parent)
{
	if (manager)
	{
		manager->Object.Description = "WS2812_DrawManagerBaseT";
		manager->Object.Parent = parent;
		manager->Template = 0;
		
		manager->Handler = (WS2812_DrawManagerHandlerT)PrivateDrawManageHandler;
		manager->EventListener = (WS2812_DrawManagerEventListenerT)PrivateDrawManageEventListener;
		manager->RequestListener = (WS2812_DrawManagerRequestListenerT)PrivateDrawManageRequestListener;
		manager->GetValue = (WS2812_DrawManagerActionGetValueT)PrivateDrawManageGetValue;
		manager->SetValue = (WS2812_DrawManagerActionSetValueT)PrivateDrawManageSetValue;
		
		manager->Options.UpdateTime = 10;
	}
	return xResultAccept;
}
//==============================================================================
