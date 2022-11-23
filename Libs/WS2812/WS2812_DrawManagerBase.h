//==============================================================================
#ifndef WS2812_DRAW_MANAGER_BASE_H
#define WS2812_DRAW_MANAGER_BASE_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
#include "Common/xTypes.h"
#include "WS2812_Color.h"
//==============================================================================
typedef enum
{
	WS2812_DrawManagerEventIdle,
	WS2812_DrawManagerEventTimeSynchronization
	
} WS2812_DrawManagerEventSelector;
//------------------------------------------------------------------------------
typedef enum
{
	WS2812_DrawManagerRequestIdle,
	WS2812_DrawManagerRequestDrawingStart,
	WS2812_DrawManagerRequestDrawingStop,
	WS2812_DrawManagerRequestSetTemplate,
	WS2812_DrawManagerRequestSetHandler
	
} WS2812_DrawManagerRequestSelector;
//------------------------------------------------------------------------------
typedef enum
{
	WS2812_DrawManagerValueIdle,
	WS2812_DrawManagerValueColor
	
} WS2812_DrawManagerValueSelector;
//------------------------------------------------------------------------------
typedef void (*WS2812_DrawManagerHandlerT)(void* device);

typedef void (*WS2812_DrawManagerEventListenerT)(void* device, WS2812_DrawManagerEventSelector selector, uint32_t args, uint32_t count);
typedef xResult (*WS2812_DrawManagerRequestListenerT)(void* device, WS2812_DrawManagerRequestSelector selector, uint32_t args, uint32_t count);

typedef uint32_t (*WS2812_DrawManagerActionGetValueT)(void* device, WS2812_DrawManagerValueSelector selector);
typedef xResult (*WS2812_DrawManagerActionSetValueT)(void* device, WS2812_DrawManagerValueSelector selector, uint32_t value);
//------------------------------------------------------------------------------
typedef enum
{
	WS2812_DrawStateIdle,
	WS2812_DrawStateCycleStart,
	WS2812_DrawStateCycleWork,
	WS2812_DrawStateCycleEnd
	
} WS2812_DrawStates;
//------------------------------------------------------------------------------
typedef struct
{
	uint32_t DrawingIsEnable : 1;
	uint32_t IsUpdate : 1;

	uint32_t DrawState : 4; //WS2812_DrawStates
	
} WS2812_DrawManagerStatusT;
//------------------------------------------------------------------------------
typedef struct
{
	uint32_t UpdateTime;
	
} WS2812_DrawManagerOptionsT;
//------------------------------------------------------------------------------
typedef struct
{
	ObjectBaseT Object;
	
	void* Template;
	
	WS2812_DrawManagerHandlerT Handler;
	WS2812_DrawManagerEventListenerT EventListener;
	WS2812_DrawManagerRequestListenerT RequestListener;
	WS2812_DrawManagerActionGetValueT GetValue;
	WS2812_DrawManagerActionSetValueT SetValue;

	WS2812_DrawManagerStatusT Status;
	WS2812_DrawManagerOptionsT Options;
	
	uint32_t UpdateTime;
	
} WS2812_DrawManagerBaseT;
//==============================================================================
xResult WS2812_DrawManagerBaseInit(WS2812_DrawManagerBaseT* manager, void* parent);

xResult WS2812_DrawManageSetColor(WS2812_DrawManagerBaseT* manager, WS2812_ColorT color);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
