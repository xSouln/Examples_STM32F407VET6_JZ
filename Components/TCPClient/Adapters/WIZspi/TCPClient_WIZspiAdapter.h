//==============================================================================
//module enable:

#include "TCPClient/Adapters/TCPClient_AdapterConfig.h"
#ifdef TCP_CLIENT_WIZ_SPI_ADAPTER_ENABLE
//==============================================================================
//header:

#ifndef _TCP_CLIENT_WIZ_SPI_ADAPTER_H
#define _TCP_CLIENT_WIZ_SPI_ADAPTER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "TCPClient/Controls/TCPClient.h"
#include "Common/xRxReceiver.h"
//==============================================================================
//types:

typedef struct
{
	void (*HardwareResetOn)();
	void (*HardwareResetOff)();

	void (*SelectChip)();
	void (*DeselectChip)();

	void (*TransmiteByte)(uint8_t byte);
	uint8_t (*ReceiveByte)();

} TCPClientWIZspiBusInterfaceT;
//------------------------------------------------------------------------------

typedef struct
{
	REG_SPI_T* SPI;
	TCPClientWIZspiBusInterfaceT BusInterface;

	xDataBufferT* ResponseBuffer;
	xRxReceiverT RxReceiver;

	uint8_t* RxBuffer;
	uint16_t RxBufferSize;

} TCPClientWIZspiAdapterT;
//==============================================================================
//functions:

xResult TCPClientWIZspiAdapterInit(TCPClientT* server, TCPClientWIZspiAdapterT* adapter);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TCP_CLIENT_WIZ_SPI_ADAPTER_H
#endif //TCP_CLIENT_WIZ_SPI_ADAPTER_ENABLE
