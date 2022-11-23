//==============================================================================
#ifndef X_TX_TRANSFER_H
#define X_TX_TRANSFER_H
//==============================================================================
//includes:

#include "xTypes.h"
#include "xTx.h"
//==============================================================================
//defines:

#define X_TX_TRANSFER_OBJECT_ID 0x907EA7FB
//==============================================================================
//types:

typedef enum
{
	xTxTransferEventTransferStarted = 1U,
	xTxTransferEventTransferComplite,
	xTxTransferEventTimeOut,
	xTxTransferEvent
	
} xTxTransferEventSelector;
//------------------------------------------------------------------------------

typedef enum
{
	xTxTransferRequestIdle,

	xTxTransferRequestDelay,
	xTxTransferRequestTransmit
	
} xTxTransferRequestSelector;
//------------------------------------------------------------------------------

typedef enum
{
	xTxTransferValueIdle,

	xTxTransferValueBufferSize,
	xTxTransferValueBufferFreeSize,
	
} xTxTransferValueSelector;
//------------------------------------------------------------------------------

typedef enum
{
	xTxTransferStatusIdle,
	xTxTransferStatusBeginning,
	xTxTransferStatusTransmits,
	xTxTransferStatusEnding,
	xTxTransferStatusComplite,
	xTxTransferStatusError
	
} xTxTransferStatus;
//------------------------------------------------------------------------------

typedef void (*xTxTransferHandlerT)(void* transfer);
typedef void (*xTxTransferEventListenerT)(void* transfer, xTxTransferEventSelector selector, uint32_t args, uint32_t count);
typedef xResult (*xTxTransferRequestListenerT)(void* transfer, xTxTransferRequestSelector selector, uint32_t args, uint32_t count);
typedef int (*xTxTransferActionGetValueT)(void* transfer, xTxTransferValueSelector selector);
//------------------------------------------------------------------------------

typedef struct
{
	xTxTransferHandlerT Handler;
	xTxTransferEventListenerT EventListener;
	xTxTransferRequestListenerT RequestListener;
	xTxTransferActionGetValueT GetValue;
	
} xTxTransferInterfaceT;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		uint32_t IsInit : 1;
		xTxTransferStatus Transfer : 4;
		uint32_t Transmitter : 4;
	};
	
	uint32_t Value;
	
} xTxTransferStatusT;
//------------------------------------------------------------------------------

typedef struct
{
	ObjectBaseT Object;
	
	xTxTransferStatusT Status;
	
	xTxT* Tx;
	void* Adapter;
	xTxTransferInterfaceT* Interface;
	
	uint8_t* Data;
	uint32_t DataSize;
	uint32_t DataTransferred;
	
	uint32_t MaxContentSize;
	uint32_t MinContentSize;
	
	float BufferFilling;
	
	uint32_t TimeOut;
	
} xTxTransferT;
//==============================================================================
//functions:

xResult xTxTransferInit(xTxTransferT* layer,
												uint32_t min_content_size,
												uint32_t max_content_size,
												float buffer_filling);
														
xResult xTxTransferStart(xTxTransferT* layer, void* data, uint32_t data_size);
xResult xTxTransferSetTxLine(xTxTransferT* layer, xTxT* tx);

void xTxTransferAbort(xTxTransferT* layer);
void xTxTransferHandler(xTxTransferT* layer);
//==============================================================================
#endif //X_TX_TRANSFER_H
