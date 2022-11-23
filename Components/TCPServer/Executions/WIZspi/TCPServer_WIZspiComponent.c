//==============================================================================
//module enable:

#include "TCPServer/TCPServer_ComponentConfig.h"
#ifdef TCP_SERVER_WIZ_SPI_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "TCPServer_WIZspiComponent.h"
#include "TCPServer/TCPServer_Component.h"
#include "TCPServer/Adapters/WIZspi/TCPServer_WIZspiAdapter.h"

#include "Components.h"

#ifdef TERMINAL_COMPONENT_ENABLE
#include "Terminal/Controls/Terminal.h"
#endif
//==============================================================================
//defines:

#define RX_BUF_SIZE TCP_SERVER_WIZ_SPI_RX_BUF_SIZE
#define RX_RECEIVER_BUF_SIZE TCP_SERVER_WIZ_SPI_RX_RECEIVER_BUF_SIZE
//==============================================================================
//variables:

static uint8_t rx_buf[RX_BUF_SIZE];
static uint8_t rx_receiver_buf[RX_RECEIVER_BUF_SIZE];

TCPServerT TCPServerWIZspi;
//==============================================================================
//functions:

void WIZspiHardwareResetOn()
{
	//WIZ_RESET_GPIO_Port->ODR &= ~WIZ_RESET_Pin;
}
//------------------------------------------------------------------------------
void WIZspiHardwareResetOff()
{
	//WIZ_RESET_GPIO_Port->ODR |= WIZ_RESET_Pin;
}
//------------------------------------------------------------------------------
void WIZspiSelectChip()
{
	//WIZ_CS_GPIO_Port->ODR &= ~WIZ_CS_Pin;
}
//------------------------------------------------------------------------------
void WIZspiDeselectChip()
{
	//WIZ_CS_GPIO_Port->ODR |= WIZ_CS_Pin;
}
//------------------------------------------------------------------------------
uint8_t WIZspiReceiveByte()
{
	uint8_t byte = 0xff;
  /*
  WIZspi->Control2.CurrentDataSize = 1;
  WIZspi->Control1.SpiEnable = true;
  WIZspi->Control1.MasterTransferStart = true;

  while(!WIZspi->Status.TxEmpty){ };
  WIZspi->TxData.Word = byte;

  while(!WIZspi->Status.TxFifoComplete){ };
  byte = WIZspi->RxData.Word;

  WIZspi->Clear.EndOfTransfer = true;
  WIZspi->Clear.TransferFilled = true;
  WIZspi->Control1.SpiEnable = false;
*/
  return byte;
}
//------------------------------------------------------------------------------
void WIZspiTransmiteByte(uint8_t byte)
{
	/*
	WIZspi->Control2.CurrentDataSize = 1;
  WIZspi->Control1.SpiEnable = true;
  WIZspi->Control1.MasterTransferStart = true;
  
  while(!WIZspi->Status.TxEmpty){ };
  WIZspi->TxData.Word = byte;
  
  while(!WIZspi->Status.TxFifoComplete){ };
  byte = WIZspi->RxData.Word;
  
  WIZspi->Clear.EndOfTransfer = true;
  WIZspi->Clear.TransferFilled = true;
  WIZspi->Control1.SpiEnable = false;
  */
}
//------------------------------------------------------------------------------
void _TCPServerWIZspiComponentEventListener(TCPServerT* server, TCPServerEventSelector selector, void* arg, ...)
{
	switch ((uint8_t)selector)
	{
		case TCPServerEventEndLine:
			break;
		
		case TCPServerEventBufferIsFull:
			break;
	}
}
//------------------------------------------------------------------------------
xResult _TCPServerWIZspiComponentRequestListener(TCPServerT* server, TCPServerRequestSelector selector, void* arg, ...)
{
	switch ((uint8_t)selector)
	{
		case TCPServerRequestDelay:
			HAL_Delay((uint32_t)arg);
			break;
		
		default : return xResultRequestIsNotFound;
	}
	
	return xResultAccept;
}
//------------------------------------------------------------------------------

void _TCPServerWIZspiComponentIRQListener(TCPServerT* port, ...)
{

}
//------------------------------------------------------------------------------
/**
 * @brief main handler
 */
inline void _TCPServerWIZspiComponentHandler()
{

}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
inline void _TCPServerWIZspiComponentTimeSynchronization()
{

}
//==============================================================================
//initializations:

TCPServerInterfaceT TCPServerInterface =
{
	INITIALIZATION_EVENT_LISTENER(TCPServer, ComponentsEventListener),
	INITIALIZATION_REQUEST_LISTENER(TCPServer, ComponentsRequestListener)
};

//------------------------------------------------------------------------------

TCPServerWIZspiAdapterT TCPServerWIZspiAdapter =
{
	.SPI = TCP_SERVER_WIZ_SPI_REG,

	.BusInterface =
	{
		.SelectChip = WIZspiSelectChip,
		.DeselectChip = WIZspiSelectChip,

		.HardwareResetOn = WIZspiHardwareResetOn,
		.HardwareResetOff = WIZspiHardwareResetOff,

		.TransmiteByte = WIZspiTransmiteByte,
		.ReceiveByte = WIZspiReceiveByte
	},
};
//==============================================================================
//initialization:

xResult TCPServerWIZspiComponentInit(void* parent)
{
	#ifdef TERMINAL_COMPONENT_ENABLE
	TCPServerWIZspiAdapter.ResponseBuffer = &Terminal.ResponseBuffer;
	#endif
	
	TCPServerWIZspiAdapter.RxBuffer = rx_buf;
	TCPServerWIZspiAdapter.RxBufferSize = sizeof(rx_buf);
	
	xRxReceiverInit(&TCPServerWIZspiAdapter.RxReceiver,
									&TCPServerWIZspi.Rx,
									0,
									rx_receiver_buf,
									sizeof(rx_receiver_buf));
	
	TCPServerInit(&TCPServerWIZspi, parent, &TCPServerInterface);
	TCPServerWIZspiAdapterInit(&TCPServerWIZspi, &TCPServerWIZspiAdapter);
	
  return xResultAccept;
}
//==============================================================================
#endif //TCP_SERVER_WIZ_SPI_COMPONENT_ENABLE
