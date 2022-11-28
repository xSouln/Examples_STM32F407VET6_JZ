//==============================================================================
#ifndef X_RX_RECEIVER_H
#define X_RX_RECEIVER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "xTypes.h"
#include "xCircleBuffer.h"
#include "xRx.h"
//==============================================================================
//defines:

#define X_RX_RECEIVER_OBJECT_ID 0xDAD6678E
//==============================================================================
//types:

typedef enum
{
	xRxReceiverEventIdle,

	xRxReceiverEventEndLine,
	xRxReceiverEventBufferIsFull
	
} xRxReceiverEventSelector;
//------------------------------------------------------------------------------

typedef enum
{
	xRxPacketReceiverValueIdle
	
} xRxReceiverValueSelector;
//------------------------------------------------------------------------------

DEFINITION_EVENT_LISTENER_TYPE(xRxReceiver, xTxEventSelector);
//------------------------------------------------------------------------------

typedef struct
{
	DECLARE_EVENT_LISTENER(xRxReceiver);
	
} xRxReceiverInterfaceT;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		uint32_t IsInit : 1;
	};
	uint32_t Value;
	
} xRxReceiverStatusT;
//------------------------------------------------------------------------------

typedef struct
{
	const void* Description;
	xRxT* Parent;
	
	xRxReceiverStatusT Status;
	xRxReceiverInterfaceT* Interface;
	
	uint8_t* Buffer;
	uint32_t BufferSize;
	uint32_t BytesReceived;
	
} xRxReceiverT;
//==============================================================================
//functions:

void xRxReceiverReceive(xRxReceiverT* receiver, uint8_t *data, uint32_t data_size);
void xRxReceiverReceiveReverce(xRxReceiverT* receiver, uint8_t *data, uint32_t data_size);

void xRxReceiverRead(xRxReceiverT* receiver, xCircleBufferT* circle_buffer);

int xRxReceiverInit(xRxReceiverT* receiver,
						xRxT* parent,
						xRxReceiverInterfaceT* interface,
						uint8_t* buffer,
						uint32_t buffer_size);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //X_RX_RECEIVER_H
