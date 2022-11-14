//==============================================================================
#ifndef X_RX_RECEIVER_H
#define X_RX_RECEIVER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
#include "xTypes.h"
#include "xCircleBuffer.h"
#include "xRx.h"
//==============================================================================
typedef enum
{
  xRxReceiverEventEndLine = 1U,
	xRxReceiverEventBufferIsFull
	
} xRxReceiverEventSelector;
//------------------------------------------------------------------------------
typedef enum
{
	xPacketReceiverValue = 1U
	
} xRxReceiverValueSelector;
//------------------------------------------------------------------------------
typedef void (*xRxReceiverEventListener)(void* receiver, xRxReceiverEventSelector event, uint32_t args, uint32_t count);
typedef uint32_t (*xRxReceiverGetValue)(void* rx, xRxReceiverValueSelector selector);
typedef int (*xRxReceiverSetValue)(void* rx, xRxReceiverValueSelector selector, uint32_t value);
//------------------------------------------------------------------------------
typedef struct
{
	xRxReceiverEventListener EventListener;
	
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
	char* Description;
	xRxT* Parent;
	
	xRxReceiverStatusT Status;
	xRxReceiverInterfaceT* Interface;
	
  uint8_t* Buffer;
  uint32_t BufferSize;
  uint32_t BytesReceived;
	
} xRxReceiverT;
//==============================================================================
void xRxReceiverReceive(xRxReceiverT* receiver, uint8_t *data, uint32_t data_size);
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
#endif /* X_RX_RECEIVER_H */
