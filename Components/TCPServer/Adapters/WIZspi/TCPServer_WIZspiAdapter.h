//==============================================================================
//header:

#ifndef TCP_SERVER_WIZ_SPI_ADAPTER_H
#define TCP_SERVER_WIZ_SPI_ADAPTER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "TCPServer/Controls/TCPServer.h"
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

} TCPServerWIZspiBusInterfaceT;
//------------------------------------------------------------------------------

typedef struct
{
	void* Server;

	REG_SPI_T* SPI;
	TCPServerWIZspiBusInterfaceT BusInterface;

	xDataBufferT* ResponseBuffer;
	xRxReceiverT RxReceiver;

	uint8_t* RxBuffer;
	uint16_t RxBufferSize;

} TCPServerWIZspiAdapterT;
//==============================================================================
//functions:

xResult TCPServerWIZspiAdapterInit(TCPServerT* server, TCPServerWIZspiAdapterT* adapter);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif /* TCP_SERVER_WIZ_SPI_ADAPTER_H */

