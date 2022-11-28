//==============================================================================
#ifndef X_RX_RECIVER_H
#define X_RX_RECIVER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//include:

#include "xTypes.h"
#include "xTx.h"
//==============================================================================
//defines:

#define X_RX_OBJECT_ID 0xCDC59799
//==============================================================================
//types:

typedef enum
{
	xRxEventIdle,

	xRxEventReceiveComplete,

	xRxEventsCount
	
} xRxEventSelector;
//------------------------------------------------------------------------------

typedef enum
{
	xRxRequestIdle,

	//xRxRequesReceive,
	xRxRequestEnableReceiver,
	xRxRequestDisableReceiver,
	
	//xRxRequestPutInResponseBuffer,
	xRxRequestClearBuffer,
	xRxRequestClearResponseBuffer,
	
	xRxRequestsCount

} xRxRequestSelector;
//------------------------------------------------------------------------------

typedef enum
{
	xRxValueIdle,

	xRxValueReceiverEnableState,
	
	//xRxValueResponseBuffer,
	//xRxValueResponseBufferSize,

	xRxValueCount
	
} xRxValueSelector;
//------------------------------------------------------------------------------

typedef union
{
  struct
	{
		uint32_t IsInit : 1;
		
		uint32_t Receiver : 4;
	};
	uint32_t Value;
	
} xRxStatusT;
//------------------------------------------------------------------------------

DEFINITION_HANDLER_TYPE(xRx);

DEFINITION_EVENT_LISTENER_TYPE(xRx, xRxEventSelector);
DEFINITION_IRQ_LISTENER_TYPE(xRx);
DEFINITION_REQUEST_LISTENER_TYPE(xRx, xRxRequestSelector);

DEFINITION_GET_VALUE_ACTION_TYPE(xRx, xRxValueSelector);
DEFINITION_SET_VALUE_ACTION_TYPE(xRx, xRxValueSelector);

typedef xResult (*xRxReceiveActionT)(void* rx, uint8_t* data, uint32_t size);
typedef xResult (*xRxPutInResponseBufferActionT)(void* rx, void* data, uint32_t size);

typedef uint8_t* (*xRxGetBufferActionT)(void* rx);
typedef uint32_t (*xRxGetBufferSizeActionT)(void* rx);
typedef uint32_t (*xRxGetBytesCountInBufferActionT)(void* rx);

typedef uint8_t* (*xRxGetResponseBufferActionT)(void* rx);
typedef uint32_t (*xRxGetResponseBufferSizeActionT)(void* rx);
typedef uint32_t (*xRxGetBytesCountInResponseBufferActionT)(void* rx);
//------------------------------------------------------------------------------

typedef struct
{
	DECLARE_HANDLER(xRx);

	DECLARE_IRQ_LISTENER(xRx);

	DECLARE_EVENT_LISTENER(xRx);
	DECLARE_REQUEST_LISTENER(xRx);

	//DECLARE_GET_VALUE_ACTION(xRx);
	//DECLARE_SET_VALUE_ACTION(xRx);
	
	xRxReceiveActionT Receive;
	xRxPutInResponseBufferActionT PutInResponseBuffer;

	xRxGetBufferActionT GetBuffer;
	xRxGetBufferSizeActionT GetBufferSize;
	xRxGetBytesCountInBufferActionT GetBytesCountInBuffer;

	xRxGetResponseBufferActionT GetResponseBuffer;
	xRxGetResponseBufferSizeActionT GetResponseBufferSize;
	xRxGetBytesCountInResponseBufferActionT GetBytesCountInResponseBuffer;

} xRxInterfaceT;
//------------------------------------------------------------------------------

typedef void xRxAdapterT;
//------------------------------------------------------------------------------

typedef struct
{
	ObjectBaseT Object;

	xRxStatusT Status;
	
	xRxAdapterT* Adapter;
	const xRxInterfaceT* Interface;
	
	xTxT* Tx;
	
} xRxT;
//==============================================================================
//macros:

#define xRxHandler(rx) ((xRxT*)rx)->Interface->Handler(rx)

#define xRxIRQListener(rx) ((xRxT*)rx)->Interface->IRQListener(rx)

#define xRxEventListener(rx, event, arg, ...) ((xRxT*)rx)->Interface->EventListener(rx, event, arg, ##__VA_ARGS__)
#define xRxRequestListener(rx, selector, arg, ...) ((xRxT*)rx)->Interface->RequestListener(rx, selector, arg, ##__VA_ARGS__)

#define xRxSetValue(rx, selector, value) ((xRxT*)rx)->Interface->SetValue(rx, selector, value)
#define xRxGetValue(rx, selector, value) ((xRxT*)rx)->Interface->GetValue(rx, selector, value)
//------------------------------------------------------------------------------

#define xRxReceive(rx, data, size) ((xRxT*)rx)->Interface->Receive(rx, data, size)
#define xRxPutInResponseBuffer(rx, data, size) ((xRxT*)rx)->Interface->PutInResponseBuffer(rx, data, size)

#define xRxGetBuffer(rx) ((xRxT*)rx)->Interface->xRxGetBuffer(rx)
#define xRxGetBufferSize(rx) ((xRxT*)rx)->Interface->GetBufferSize(rx)
#define xRxGetBytesCountInBuffer(rx) ((xRxT*)rx)->Interface->GetBytesCountInBuffer(rx)
#define xRxClearBuffer(rx) ((xRxT*)rx)->Interface->RequestListener(rx, xRxRequestClearBuffer, 0)

#define xRxGetResponseBuffer(rx) ((xRxT*)rx)->Interface->GetResponseBuffer(rx)
#define xRxGetResponseBufferSize(rx) ((xRxT*)rx)->Interface->GetResponseBufferSize(rx)
#define xRxGetBytesCountInResponseBuffer(rx) ((xRxT*)rx)->Interface->GetBytesCountInResponseBuffer(rx)
#define xRxClearResponseBuffer(rx) ((xRxT*)rx)->Interface->RequestListener(rx, xRxRequestClearResponseBuffer, 0)

#define xRxTxBind(rx, tx) (((xRxT*)rx)->Tx = tx)
//==============================================================================
//functions:

xResult xRxInit(xRxT* rx, void* parent, xTxAdapterT* adapter, const xRxInterfaceT* interface);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
