//==============================================================================
#ifndef X_RX_REQUEST_H
#define X_RX_REQUEST_H
//==============================================================================
#include "xTypes.h"
#include "xRx.h"
//==============================================================================
typedef xResult (*xRxRequestReceiverT)(xObject manager, //xRequestManagerT
																				xObject object,
																				uint16_t object_size);
//------------------------------------------------------------------------------
typedef struct
{
  void* Header;
  uint8_t HeaderLength;
  xRxRequestReceiverT Action;
	void* Content;
   
} xRxRequestT;
//------------------------------------------------------------------------------
typedef struct
{
	void* Device;
	
	void* Adapter;
	
  xRxRequestT* Requests;
	xRxRequestT* FoundRequest;
	
	xRxT* RxLine;
   
} xRxRequestManagerT;
//==============================================================================
xRxRequestT* xRxRequestIdentify(xRxT* rx, void* device, xRxRequestT* requests, uint8_t data[], uint16_t data_size);
xResult xRxRequestManagerInit(xRxRequestManagerT* manager, void* device, xRxRequestT* requests);
//==============================================================================
#define NEW_RX_REQUEST0(header, request)\
{\
  .Header = (char*)header,\
  .HeaderLength = SIZE_STRING(header),\
  .Action = (xRxRequestReceiverT)request\
}
//------------------------------------------------------------------------------
#define NEW_RX_REQUEST1(header, request)\
{\
  .Header = (char*)&header,\
  .HeaderLength = sizeof(header),\
  .Action = (xRxRequestReceiverT)request\
}
//==============================================================================
#endif /* X_RECEIVER_REQUEST_H */
