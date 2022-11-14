//==============================================================================
#ifndef _TERMINAL_TYPES_H
#define _TERMINAL_TYPES_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components_Types.h"
#include "Terminal_Config.h"
#include "Terminal_Info.h"
//==============================================================================
//types:

typedef enum
{
	TerminalEventIdle,
	
} TerminalEventSelector;
//------------------------------------------------------------------------------

typedef enum
{
	TerminalRequestIdle,
	
} TerminalRequestSelector;
//------------------------------------------------------------------------------

typedef enum
{
	TerminalValueIdle,
	
} TerminalValueSelector;
//------------------------------------------------------------------------------

typedef void (*TerminalHandlerT)(void* device);
typedef void (*TerminalEventListenerT)(void* device, TerminalEventSelector event, uint32_t args, uint32_t count);

typedef xResult (*TerminalRequestListenerT)(void* device, TerminalRequestSelector event, uint32_t args, uint32_t count);

typedef struct
{
	TerminalHandlerT Handler;
	
	TerminalEventListenerT EventListener;
	TerminalRequestListenerT RequestListener;
	
} TerminalInterfaceT;
//------------------------------------------------------------------------------

typedef enum
{
	DeviceEventIdle,
	
} DeviceEventSelector;
//------------------------------------------------------------------------------
typedef enum
{
	DeviceRequestIdle,
	
} DeviceRequestSelector;
//------------------------------------------------------------------------------

typedef enum
{
	DeviceValueDeviceInfo,
	
} DeviceValueSelector;
//------------------------------------------------------------------------------

typedef struct
{
	OBJECT_HEADER;

	TerminalInterfaceT Interface;

} TerminalT;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif /* TYPES_H */
