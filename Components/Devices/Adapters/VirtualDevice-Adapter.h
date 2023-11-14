//==============================================================================
//header:

#ifndef _SERVER_DEVICE_ADAPTER_H_
#define _SERVER_DEVICE_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xDevice/xDevice.h"
#include "Abstractions/xPort/xPort.h"
#include "Abstractions/xTransferLayer/xTransferLayer-Types.h"
#include "CAN_Local/Control/CAN_Local-Types.h"
#include "Common/xCircleBuffer.h"
#include "Services/GAP/Adapters/GAPService-Adapter.h"
#include "Services/GAP/Adapters/VirtualGAPService-Adapter.h"
//==============================================================================
//types:

typedef struct
{
	uint32_t OperationTimeStamp;
	uint16_t OperationTimeOut;

	uint16_t RxPacketHandlerIndex;

	uint16_t ServicesInitState;

	uint8_t ServicesCount;
	uint8_t TotalServiceNumber;

	VirtualGAPServiceAdapterT GAPAdapter;
	GAPServiceT GAP;

	//CAN_LocalRequestT* Request;

	//void* OperationHandle;

} VirtualDeviceAdapterContentT;
//------------------------------------------------------------------------------
typedef struct
{
	VirtualDeviceAdapterContentT Content;

	//xPortT* Port;
	//xTransferLayerT* TransferLayer;

} VirtualDeviceAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	xPortT* Port;
	xTransferLayerT* TransferLayer;

} VirtualDeviceAdapterInitT;
//==============================================================================
//functions:

xResult VirtualDeviceAdapterInit(xDeviceT* device, VirtualDeviceAdapterT* adapter, VirtualDeviceAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SERVER_DEVICE_ADAPTER_H_
