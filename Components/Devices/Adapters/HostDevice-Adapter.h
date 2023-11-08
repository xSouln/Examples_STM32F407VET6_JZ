//==============================================================================
//header:

#ifndef _HOST_DEVICE_ADAPTER_H_
#define _HOST_DEVICE_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xDevice/xDevice.h"
#include "Abstractions/xPort/xPort.h"
#include "Abstractions/xTransferLayer/xTransferLayer-Types.h"
#include "Common/xCircleBuffer.h"
//==============================================================================
//types:

typedef struct
{
	uint32_t RxPacketHandlerIndex;
	xCircleBufferT* PortRxCircleBuffer;

} HostDeviceAdapterContentT;
//------------------------------------------------------------------------------
typedef struct
{
	HostDeviceAdapterContentT Content;

	xPortT* Port;
	xTransferLayerT* TransferLayer;

} HostDeviceAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	xPortT* Port;
	xTransferLayerT* TransferLayer;

} HostDeviceAdapterInitT;
//==============================================================================
//functions:

xResult HostDeviceAdapterInit(xDeviceT* device, HostDeviceAdapterT* adapter, HostDeviceAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_HOST_DEVICE_ADAPTER_H_
