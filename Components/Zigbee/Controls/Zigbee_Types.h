//==============================================================================
#ifndef _ZIGBEE_TYPES_H
#define _ZIGBEE_TYPES_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components_Types.h"
#include "Common/xTx.h"
#include "Common/xRx.h"
#include "Common/xRxReceiver.h"
#include "Zigbee_Config.h"
#include "Zigbee_Info.h"
#include "Zigbee/Adapters/Zigbee_AdapterBase.h"
//==============================================================================
//defines:

//==============================================================================
//types:

typedef enum
{
	ZigbeeEventIdle,

	ZigbeeEventEndLine,
	ZigbeeEventBufferIsFull,
	
} ZigbeeEventSelector;
//------------------------------------------------------------------------------

typedef struct
{
	uint8_t* Data;
	uint32_t Size;

} ZigbeeReceivedDataT;
//------------------------------------------------------------------------------

typedef enum
{
	ZigbeeRequestIdle,

} ZigbeeRequestSelector;
//------------------------------------------------------------------------------
DEFINITION_EVENT_LISTENER_TYPE(Zigbee, ZigbeeEventSelector);
DEFINITION_REQUEST_LISTENER_TYPE(Zigbee, ZigbeeRequestSelector);
//------------------------------------------------------------------------------

typedef struct
{
	DECLARE_EVENT_LISTENER(Zigbee);
	DECLARE_REQUEST_LISTENER(Zigbee);

} ZigbeeInterfaceT;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		xResult InitResult : 4;
		xResult AdapterInitResult : 4;
		xResult RxInitResult : 4;
		xResult TxInitResult : 4;
		
	};
	uint32_t Value;
	  
} ZigbeeStatusT;
//------------------------------------------------------------------------------

typedef struct
{
	uint16_t UpdateTime;

} ZigbeeOptionsT;
//------------------------------------------------------------------------------

typedef struct
{
	ObjectBaseT Object;
	
	ZigbeeAdapterBaseT Adapter;
	
	ZigbeeInterfaceT* Interface;
	
	ZigbeeStatusT Status;

	ZigbeeOptionsT Options;

	xRxT Rx;
	xTxT Tx;
	  
} ZigbeeT;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_ZIGBEE_TYPES_H
