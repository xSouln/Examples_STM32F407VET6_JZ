//==============================================================================
#ifndef X_TX_H
#define X_TX_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "xTypes.h"
#include "xDataBuffer.h"
#include "xCircleBuffer.h"
//==============================================================================
//types:

#define xtx_word_t uint32_t

#define X_TX_OBJECT_ID 0x642BF0
//------------------------------------------------------------------------------

typedef enum
{
	xTxEventIdle,

	xTxEventStartTransmission,
	xTxEventStopTransmission,
	xTxEventAbortTransmission,
	xTxEventTransmissionComplete,

	xTxEventsCount
	
} xTxEventSelector;
//------------------------------------------------------------------------------

typedef enum
{
	xTxRequestIdle,

	//xTxRequestTransmitData,
	xTxRequestUpdateTransmitterStatus,

	xTxRequestEnableTransmitter,
	xTxRequestDisableTransmitter,
	
	xTxRequestClearBuffer,
	
	xTxRequestCount

} xTxRequestSelector;
//------------------------------------------------------------------------------

typedef enum
{
	xTxValueIdle,

	//xTxValueTransmitterStatus,
	//xTxValueBufferSize,
	//xTxValueFreeBufferSize,
	
	xTxValuesCount

} xTxValueSelector;
//------------------------------------------------------------------------------

typedef enum
{
	xTxStatusIdle,

	xTxStatusIsTransmits,
	xTxStatusError

} xTxTransmitterStatus;
//------------------------------------------------------------------------------

DEFINITION_HANDLER_TYPE(xTx);

DEFINITION_EVENT_LISTENER_TYPE(xTx, xTxEventSelector);
DEFINITION_IRQ_LISTENER_TYPE(xTx);
DEFINITION_REQUEST_LISTENER_TYPE(xTx, xTxRequestSelector);

DEFINITION_GET_VALUE_ACTION_TYPE(xTx, xTxValueSelector);
DEFINITION_SET_VALUE_ACTION_TYPE(xTx, xTxValueSelector);

typedef xResult (*xTxTransmitDataT)(void* tx, void* data, uint32_t data_size);

typedef uint32_t (*xTxGetBufferSizeActionT)(void* tx);
typedef uint32_t (*xTxGetFreeBufferSizeActionT)(void* tx);
//------------------------------------------------------------------------------

typedef struct
{
	DECLARE_HANDLER(xTx);

	DECLARE_IRQ_LISTENER(xTx);

	DECLARE_EVENT_LISTENER(xTx);
	DECLARE_REQUEST_LISTENER(xTx);

	xTxTransmitDataT TransmitData;

	xTxGetBufferSizeActionT GetBufferSize;
	xTxGetFreeBufferSizeActionT GetFreeBufferSize;

} xTxInterfaceT;
//------------------------------------------------------------------------------

typedef union
{
  struct
  {
    uint32_t Transmitted : 1;
		
		uint32_t RequestState : 4;
	};
	
  uint32_t Value;
	
} xTxHandleT;
//------------------------------------------------------------------------------

typedef union
{
  struct
  {
		uint32_t IsInit : 1;
		
		xTxTransmitterStatus Transmitter : 4;
  };
	
  uint32_t Value;
	
} xTxStatusT;
//------------------------------------------------------------------------------

typedef void xTxAdapterT;
//------------------------------------------------------------------------------

typedef struct
{
	ObjectBaseT Object;
	
	xTxHandleT Handle;
	xTxStatusT Status;
	
	xTxAdapterT* Adapter;
	const xTxInterfaceT* Interface;
	
} xTxT;
//==============================================================================
//functions:

xResult _xTxTransmitByte(xTxT *tx, uint8_t byte);
xResult _xTxTransmitWord(xTxT* tx, uint32_t data);
xResult _xTxTransmitString(xTxT *tx, char* str);

xResult xTxInit(xTxT* tx, void* parent, xTxAdapterT* adapter, xTxInterfaceT* interface);
//==============================================================================
//macros:

#define xTxHandler(tx) ((xTxT*)tx)->Interface->Handler(tx)

#define xTxIRQListener(tx) ((xTxT*)tx)->Interface->IRQListener(tx)

#define xTxEventListener(tx, event, arg, ...) ((xTxT*)tx)->Interface->EventListener(tx, event, arg, ##__VA_ARGS__)
#define xTxRequestListener(tx, selector, arg, ...) ((xTxT*)tx)->Interface->RequestListener(tx, selector, arg, ##__VA_ARGS__)

#define xTxSetValue(tx, selector, value) ((xTxT*)tx)->Interface->SetValue(tx, selector, value)
//#define xTxGetValue(tx, selector, value) ((xTxT*)tx)->Interface->GetValue(tx, selector, value)
//------------------------------------------------------------------------------

#define xTxTransmitData(tx, data, data_size) ((xTxT*)tx)->Interface->TransmitData(tx, data, data_size)

#define xTxTransmitByte(tx, data) _xTxTransmitByte(tx, data)
#define xTxTransmitWord(tx, data) _xTxTransmitWord(tx, data)
#define xTxTransmitString(tx, str) xTxTransmitData(tx, (uint8_t*)str, strlen((char*)str))

#define xTxGetBuffer(tx) ((xTxT*)tx)->Interface->GetBuffer(tx)
#define xTxGetBufferSize(tx) ((xTxT*)tx)->Interface->GetBufferSize(tx)
#define xTxGetFreeBufferSize(tx) ((xTxT*)tx)->Interface->GetFreeBufferSize(tx)

#define xTxUpdateTransmitterStatus(tx) ((xTxT*)tx)->Interface->RequestListener(tx, xTxRequestUpdateTransmitterStatus, 0)

#define xTxClearBuffer(tx) ((xTxT*)tx)->Interface->RequestListener(tx, xTxRequestClearBuffer, 0)
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
