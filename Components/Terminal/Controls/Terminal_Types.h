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
#include "Common/xTxTransfer.h"
#include "Common/xRxRequest.h"
//==============================================================================
//types:

typedef enum
{
	TerminalEventIdle,
	
	TerminalEventTime_1000ms,

} TerminalEventSelector;
//------------------------------------------------------------------------------

typedef enum
{
	TerminalRequestIdle,
	
	TerminalRequestReceiveData,


} TerminalRequestSelector;
//------------------------------------------------------------------------------

typedef struct
{
	uint8_t* Data;
	uint32_t Size;

} TerminalRequestReceiveDataArgT;
//------------------------------------------------------------------------------

typedef enum
{
	TerminalValueIdle,
	
} TerminalValueSelector;
//------------------------------------------------------------------------------

DEFINITION_HANDLER_TYPE(Terminal);
DEFINITION_EVENT_LISTENER_TYPE(Terminal, TerminalEventSelector);
DEFINITION_REQUEST_LISTENER_TYPE(Terminal, TerminalRequestSelector);
//------------------------------------------------------------------------------

typedef struct
{
	DECLARE_HANDLER(Terminal);
	
	DECLARE_EVENT_LISTENER(Terminal);
	DECLARE_REQUEST_LISTENER(Terminal);
	
} TerminalInterfaceT;
//------------------------------------------------------------------------------

typedef struct
{
	ObjectBaseT* Object;
	xRxRequestT* Requests;
	
} TerminalDeviceT;
//------------------------------------------------------------------------------

typedef struct
{
	TerminalDeviceT* Devices;
	uint16_t DevicesCount;
	
} TerminalDeviceControlT;
//------------------------------------------------------------------------------

typedef struct
{
	ObjectBaseT Object;

	TerminalInterfaceT Interface;

	TerminalDeviceT* Devices;
	uint16_t DevicesCount;

	xDataBufferT ResponseBuffer;

	xTxTransferT Transfer;

} TerminalT;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TERMINAL_TYPES_H
