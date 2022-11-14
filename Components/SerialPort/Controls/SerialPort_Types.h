//==============================================================================
#ifndef SERIAL_PORT_TYPES_H
#define SERIAL_PORT_TYPES_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components_Types.h"
#include "Common/xTx.h"
#include "Common/xRx.h"
#include "SerialPort_Config.h"
#include "SerialPort_Info.h"
//==============================================================================
//types:

typedef enum
{
	SerialPortEventIdle,

	SerialPortEventEndLine,
	SerialPortEventReceiverBufferIsFull,
	
} SerialPortEventSelector;
//------------------------------------------------------------------------------

#define SERIAL_PORT_EVENT_BASE\
	SerialPortEventSelector Selector
//------------------------------------------------------------------------------

typedef struct
{
	SERIAL_PORT_EVENT_BASE;

} SerialPortEventBaseT;
//------------------------------------------------------------------------------

typedef void (*SerialPortEventListenerT)(void* rx, SerialPortEventBaseT* event);
//------------------------------------------------------------------------------

typedef struct
{
	SerialPortEventListenerT EventListener;
	  
} SerialPortAdapterInterfaceT;
//------------------------------------------------------------------------------

typedef struct
{
	SerialPortEventListenerT EventListener;
	  
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
	OBJECT_HEADER;
	
	void* Adapter;
	
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
#endif /* SERIAL_PORT_TYPES_H */
