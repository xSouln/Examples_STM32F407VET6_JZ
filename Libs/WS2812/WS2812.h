//==============================================================================
#ifndef WS2812_H
#define WS2812_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
#include <stdint.h>
#include <stdbool.h>
#include "WS2812_Config.h"
#include "WS2812_Info.h"
#include "WS2812_Color.h"
#include "WS2812_DrawManagerBase.h"
//==============================================================================
typedef enum
{
	#ifndef WS2812_ResultAcceptValue
	WS2812_ResultAccept = 0,
	#else
	WS2812_ResultAccept = WS2812_ResultAcceptValue,
	#endif
	
	#ifndef WS2812_ResultErrorValue
	WS2812_ResultError,
	#else
	WS2812_ResultError = WS2812_ResultErrorValue,
	#endif

	#ifndef WS2812_ResultBusyValue
	WS2812_ResultBusy,
	#else
	WS2812_ResultBusy = WS2812_ResultBusyValue,
	#endif

	#ifndef WS2812_ResultNullPointerValue
	WS2812_ResultNullPointer,
	#else
	WS2812_ResultNullPointer = WS2812_ResultNullPointerValue,
	#endif

	#ifndef WS2812_ResultInvalidParameterValue
	WS2812_ResultInvalidParameter,
	#else
	WS2812_ResultInvalidParameter = WS2812_ResultInvalidParameterValue,
	#endif
	
} WS2812_Results;
//------------------------------------------------------------------------------
#ifndef WS2812_Result
#define WS2812_Result int8_t
#endif
//==============================================================================
#define WS2812_BufT uint16_t
//==============================================================================
typedef enum
{
	WS2812_TransmitterStopped,
	WS2812_TransmitterIsTransmit,
	WS2812_TransmitterError,
	WS2812_TransmitterUndefined
	
} WS2812_TransmitterStatus;
//------------------------------------------------------------------------------
typedef enum
{
	WS2812_StateDisable,
	WS2812_StateEnable,
	
} WS2812_State;
//------------------------------------------------------------------------------
typedef enum
{
	WS2812_EventTransferComplite,
	
} WS2812_EventSelector;
//------------------------------------------------------------------------------
typedef enum
{
	WS2812_RequestDelay,
	WS2812_RequestDrawingManagerHandler,
	
	WS2812_RequestTransmitterEnable,
	WS2812_RequestTransmitterDisable,
	
} WS2812_RequestSelector;
//------------------------------------------------------------------------------
typedef enum
{
	WS2812_ValuePeriod,
	WS2812_ValueTransmitterStatus,
	WS2812_ValueTransmitterEnableState
	
} WS2812_ValueSelector;
//------------------------------------------------------------------------------
typedef enum
{
	WS2812_PixelAddModeFill,
	WS2812_PixelAddModePut = 0xffffffff,
	
} WS2812_PixelAddMode;
//------------------------------------------------------------------------------
#define WS2812_BITS_IN_PIXEL (sizeof(WS2812_ColorT) * 8)
//------------------------------------------------------------------------------
typedef void (*WS2812_ActionHandler)(void* driver);

typedef void (*WS2812_EventListener)(void* driver, WS2812_EventSelector event, uint32_t args, uint32_t count);
typedef WS2812_Result (*WS2812_RequestListener)(void* driver, WS2812_RequestSelector selector, uint32_t args, uint32_t count);

typedef int (*WS2812_ActionGetValue)(void* driver, WS2812_ValueSelector selector);
typedef WS2812_Result (*WS2812_ActionSetValue)(void* driver, WS2812_ValueSelector selector, uint32_t value);
//------------------------------------------------------------------------------
typedef struct
{
	WS2812_ActionHandler Handler;
	
	WS2812_EventListener EventListener;
	WS2812_RequestListener RequestListener;
	
	WS2812_ActionGetValue GetValue;
	WS2812_ActionSetValue SetValue;
	
} WS2812_InterfaceT;
//------------------------------------------------------------------------------
typedef union
{
	struct
	{
		uint32_t DriverInit : 1;
		uint32_t ErrorState : 3;
		uint32_t DrawingIsEnable : 1;
		
		WS2812_TransmitterStatus Transmitter : 3;
	};
	uint32_t Value;
	
} WS2812_StatusT;
//------------------------------------------------------------------------------
typedef union
{
	struct
	{
		uint32_t Free : 3;
	};
	uint32_t Value;
	
} WS2812_HandleT;
//------------------------------------------------------------------------------
typedef struct
{
	char* Description;
	void* Parent;
	
	WS2812_StatusT Status;
	WS2812_HandleT Handle;
	
	WS2812_DrawManagerBaseT DrawManager;
	
	WS2812_AdapterT* Adapter;
	WS2812_InterfaceT* Interface;
	
	WS2812_BufT* Buffer;
	uint16_t BufferSize;
	
} WS2812_T;
//==============================================================================
void WS2812_Handler(WS2812_T* driver);
void WS2812_TimeSynchronization(WS2812_T* driver);

void WS2812_DeclareEvent(WS2812_T* driver, WS2812_EventSelector selector, uint32_t args, uint32_t count);
WS2812_Result WS2812_DeclareRequest(WS2812_T* driver, WS2812_RequestSelector selector, uint32_t args, uint32_t count);

WS2812_Result WS2812_SetValue(WS2812_T* driver, WS2812_ValueSelector selector, uint32_t value);
int WS2812_GetValue(WS2812_T* driver, WS2812_ValueSelector selector);
//------------------------------------------------------------------------------
void WS2812_Draw(WS2812_T* driver);

uint16_t WS2812_SetPixel(WS2812_T* driver, WS2812_ColorT pixel, uint16_t position);
uint16_t WS2812_FillPixels(WS2812_T* driver, WS2812_ColorT pixel, uint16_t start_position, uint16_t pixels_count);
uint16_t WS2812_SetPixels(WS2812_T* driver, WS2812_ColorT* pixels, uint16_t start_position, uint16_t pixels_count);

WS2812_Result WS2812_UpdateLayout(WS2812_T* driver);

void WS2812_DrawManagerHandler(WS2812_T* driver);
xResult WS2812_DrawingStart(WS2812_T* driver, void* template_request);
void WS2812_DrawingStop(WS2812_T* driver);
xResult WS2812_DrawManagerSetTemplate(WS2812_T* driver, void* value);

WS2812_TransmitterStatus WS2812_GetTransmitterStatus(WS2812_T* driver);
//------------------------------------------------------------------------------
WS2812_Result WS2812_Init(WS2812_T* driver,
													void* parent,
													WS2812_AdapterT* adapter,
													WS2812_InterfaceT* interface,
													WS2812_BufT* buffer,
													uint16_t buffer_size);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif /* WS2812_H */
