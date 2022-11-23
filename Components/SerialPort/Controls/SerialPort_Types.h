//==============================================================================
#ifndef _SERIAL_PORT_TYPES_H
#define _SERIAL_PORT_TYPES_H
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
#include "SerialPort_Config.h"
#include "SerialPort_Info.h"
#include "SerialPort/Adapters/SerialPort_AdapterBase.h"
//==============================================================================
//defines:

//==============================================================================
//types:

typedef enum
{
	SerialPortEventIdle,

	SerialPortEventEndLine,
	SerialPortEventBufferIsFull,
	
} SerialPortEventSelector;
//------------------------------------------------------------------------------

typedef struct
{
	uint8_t* Data;
	uint32_t Size;

} SerialPortReceivedDataT;
//------------------------------------------------------------------------------

typedef enum
{
	SerialPortRequestIdle,

} SerialPortRequestSelector;
//------------------------------------------------------------------------------
DEFINITION_EVENT_LISTENER_TYPE(SerialPort, SerialPortEventSelector);
DEFINITION_REQUEST_LISTENER_TYPE(SerialPort, SerialPortRequestSelector);
//------------------------------------------------------------------------------

typedef struct
{
	DECLARE_EVENT_LISTENER(SerialPort);
	DECLARE_REQUEST_LISTENER(SerialPort);

} SerialPortInterfaceT;
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
	  
} SerialPortStatusT;
//------------------------------------------------------------------------------

typedef struct
{
	ObjectBaseT Object;
	
	SerialPortAdapterBaseT Adapter;
	
	SerialPortInterfaceT* Interface;
	
	SerialPortStatusT Status;

	xRxT Rx;
	xTxT Tx;
	  
} SerialPortT;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SERIAL_PORT_TYPES_H
