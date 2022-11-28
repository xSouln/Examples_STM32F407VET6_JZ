//==============================================================================
//module enable:

#include "TCPClient/TCPClient_ComponentConfig.h"
#ifdef TCP_CLIENT_WIZ_SPI_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "TCPClient_WIZspiComponent.h"
#include "TCPClient/TCPClient_Component.h"
#include "TCPClient/Adapters/WIZspi/TCPClient_WIZspiAdapter.h"

#include "Components.h"

#ifdef TERMINAL_COMPONENT_ENABLE
#include "Terminal/Controls/Terminal.h"
#endif
//==============================================================================
//defines:

#define RX_BUF_SIZE TCP_CLIENT_WIZ_SPI_RX_BUF_SIZE
#define RX_RECEIVER_BUF_SIZE TCP_CLIENT_WIZ_SPI_RX_RECEIVER_BUF_SIZE
//==============================================================================
//variables:

static uint8_t rx_buf[RX_BUF_SIZE];
static uint8_t rx_receiver_buf[RX_RECEIVER_BUF_SIZE];

TCPClientT TCPClientWIZspi;
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
void _TCPClientWIZspiComponentEventListener(TCPClientT* server, TCPClientEventSelector selector, void* arg, ...)
{
	switch ((uint8_t)selector)
	{
		case TCPClientEventEndLine:
			break;
		
		case TCPClientEventBufferIsFull:
			break;
	}
}
//------------------------------------------------------------------------------
xResult _TCPClientWIZspiComponentRequestListener(TCPClientT* server, TCPClientRequestSelector selector, void* arg, ...)
{
	switch ((uint8_t)selector)
	{
		case TCPClientRequestDelay:
			HAL_Delay((uint32_t)arg);
			break;
		
		default : return xResultRequestIsNotFound;
	}
	
	return xResultAccept;
}
//------------------------------------------------------------------------------

void _TCPClientWIZspiComponentIRQListener(TCPClientT* port, ...)
{

}
//------------------------------------------------------------------------------
/**
 * @brief main handler
 */
inline void _TCPClientWIZspiComponentHandler()
{

}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
inline void _TCPClientWIZspiComponentTimeSynchronization()
{

}
//==============================================================================
//initializations:

TCPClientInterfaceT TCPClientInterface =
{
	INITIALIZATION_EVENT_LISTENER(TCPClient, ComponentsEventListener),
	INITIALIZATION_REQUEST_LISTENER(TCPClient, ComponentsRequestListener)
};

//------------------------------------------------------------------------------

TCPClientWIZspiAdapterT TCPClientWIZspiAdapter =
{
	.SPI = TCP_CLIENT_WIZ_SPI_REG,

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

xResult TCPClientWIZspiComponentInit(void* parent)
{
	#ifdef TERMINAL_COMPONENT_ENABLE
	TCPClientWIZspiAdapter.ResponseBuffer = &Terminal.ResponseBuffer;
	#endif
	
	TCPClientWIZspiAdapter.RxBuffer = rx_buf;
	TCPClientWIZspiAdapter.RxBufferSize = sizeof(rx_buf);
	
	xRxReceiverInit(&TCPClientWIZspiAdapter.RxReceiver,
									&TCPClientWIZspi.Rx,
									0,
									rx_receiver_buf,
									sizeof(rx_receiver_buf));
	
	TCPClientInit(&TCPClientWIZspi, parent, &TCPClientInterface);
	TCPClientWIZspiAdapterInit(&TCPClientWIZspi, &TCPClientWIZspiAdapter);
	
  return xResultAccept;
}
//==============================================================================
#endif //TCP_CLIENT_WIZ_SPI_COMPONENT_ENABLE
