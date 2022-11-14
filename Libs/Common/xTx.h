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
//------------------------------------------------------------------------------

typedef enum
{
	xTxStateDisable,
	xTxStateEnable,
	
} xTxState;
//------------------------------------------------------------------------------

typedef enum
{
	xTxEventIRQ = 1U,
	xTxEventTransmissionComplete
	
} xTxEventSelector;
//------------------------------------------------------------------------------

typedef enum
{
	xTxRequestTransmitData = 1U,
	xTxRequestEnableTransmitter,
	xTxRequestDisableTransmitter,
	
	xTxRequestStartTransmission,
	xTxRequestStopTransmission,
	xTxRequestAbortTransmission,
	
	xTxRequestClearBuffer,
	
	xTxRequestAddToIntermediateBuffer
	
} xTxRequestSelector;
//------------------------------------------------------------------------------

typedef enum
{
	xTxValueTransmitterStatus = 1U,
	xTxValueBufferSize,
	xTxValueFreeBufferSize,
	
} xTxValueSelector;
//------------------------------------------------------------------------------

typedef void (*xTxHandlerT)(void* tx);

typedef void (*xTxEventListenerT)(void* tx, xTxEventSelector event, uint32_t args, uint32_t count);
typedef xResult (*xTxRequestListenerT)(void* tx, xTxRequestSelector selector, uint32_t args, uint32_t count);

typedef int (*xTxActionGetValueT)(void* tx, xTxValueSelector selector);
typedef xResult (*xTxActionSetValueT)(void* tx, xTxValueSelector selector, uint32_t value);
//------------------------------------------------------------------------------

typedef enum
{
	xTxStatusIdle,
	xTxStatusIsTransmits,
	xTxStatusError
	
} xTxTransmitterStatus;
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

typedef struct
{
	xTxHandlerT Handler;
	
	xTxEventListenerT EventListener;
	xTxRequestListenerT RequestListener;
	
	xTxActionGetValueT GetValue;
	xTxActionSetValueT SetValue;
	
} xTxInterfaceT;
//------------------------------------------------------------------------------

typedef void xTxAdapterT;
//------------------------------------------------------------------------------

typedef struct
{
  OBJECT_HEADER;
	
  xTxHandleT Handle;
	xTxStatusT Status;
	
	xTxAdapterT* Adapter;
	xTxInterfaceT* Interface;
	
} xTxT;
//==============================================================================								
extern void xTxHandler(xTxT* tx);

extern void xTxEventListener(xTxT* tx, xTxEventSelector event, uint32_t args, uint32_t count);
extern xResult xTxRequestListener(xTxT* tx, xTxRequestSelector selector, uint32_t args, uint32_t count);

extern xResult xTxSetValue(xTxT* tx, xTxValueSelector selector, uint32_t value);
extern int xTxGetValue(xTxT* tx, xTxValueSelector selector);

extern int xTxTransmitData(xTxT* tx, void* data, uint32_t data_size);
extern int xTxTransmitByte(xTxT* tx, uint8_t byte);
extern int xTxTransmitWord(xTxT* tx, uint32_t data);
extern int xTxTransmitString(xTxT* tx, char* str);

xResult xTxInit(xTxT* tx,
								void* parent,
								xTxAdapterT* adapter,
								xTxInterfaceT* interface);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
