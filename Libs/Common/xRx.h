//==============================================================================
#ifndef X_RX_RECIVER_H
#define X_RX_RECIVER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
#include "xTypes.h"
#include "xTx.h"
//==============================================================================
typedef enum
{
	xRxEventIRQ = 1U,
	xRxEventReceiveComplete
	
} xRxEventSelector;
//------------------------------------------------------------------------------
typedef enum
{
	xRxRequestReceive = 1U,
	xRxRequestEnableReceiver,
	xRxRequestDisableReceiver,
	
	xRxRequestClearBuffer,
	
	xRxRequestClearResponseBuffer,
	xRxRequestPutInResponseBuffer,
	
} xRxRequestSelector;
//------------------------------------------------------------------------------
typedef enum
{
	xRxValueReceiverEnableState = 1U,
	
	xRxValueResponseData,
	xRxValueResponseDataSize,
	
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
typedef void (*xRxHandlerT)(void* rx);

typedef void (*xRxEventListenerT)(void* rx, xRxEventSelector event, uint32_t args, uint32_t count);
typedef xResult (*xRxRequestListenerT)(void* rx, xRxRequestSelector selector, uint32_t args, uint32_t count);

typedef int (*xRxActionGetValueT)(void* rx, xRxValueSelector selector);
typedef xResult (*xRxActionSetValueT)(void* rx, xRxValueSelector selector, uint32_t value);
//------------------------------------------------------------------------------
typedef struct
{
	xRxHandlerT Handler;
	
	xRxEventListenerT EventListener;
	xRxRequestListenerT RequestListener;
	
	xRxActionGetValueT GetValue;
	xRxActionSetValueT SetValue;
	
} xRxInterfaceT;
//------------------------------------------------------------------------------
typedef void xRxAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
  OBJECT_HEADER;
  
	xRxStatusT Status;
	
	xRxAdapterT* Adapter;
	xRxInterfaceT* Interface;
	
  xTxT* Tx;
	
} xRxT;
//==============================================================================
extern void xRxHandler(xRxT* rx);
extern int xRxReceive(xRxT* rx, uint8_t* data, uint32_t size);

extern void xRxEventListener(xRxT* rx, xRxEventSelector event, uint32_t args, uint32_t count);
extern xResult xRxRequestListener(xRxT* rx, xRxRequestSelector selector, uint32_t args, uint32_t count);

extern xResult xRxSetValue(xRxT* rx, xRxValueSelector selector, uint32_t value);
extern int xRxGetValue(xRxT* rx, xRxValueSelector selector);

extern xResult xRxPutInResponseBuffer(xRxT* rx, void* data, uint32_t size);
extern xResult xRxClearResponseBuffer(xRxT* rx);

xResult xRxInit(xRxT* rx,
									void* parent,
									xTxAdapterT* adapter,
									xRxInterfaceT* interface);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
