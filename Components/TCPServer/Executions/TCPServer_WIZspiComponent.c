//==============================================================================
//module enable:

#include "TCPServer/TCPServer_ComponentConfig.h"
#ifdef TCP_SERVER_WIZ_SPI_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "TCPServer_Component.h"
//==============================================================================
#define RX_BUF_SIZE 0x100
#define RX_RECEIVER_BUF_SIZE 0x200
//------------------------------------------------------------------------------
static uint8_t rx_buf[RX_BUF_SIZE];
static uint8_t rx_receiver_buf[RX_RECEIVER_BUF_SIZE];

REG_SPI_T* WIZspi;
//------------------------------------------------------------------------------
extern void RequestsReceiver(xRxT* rx, uint8_t* data, uint32_t size);
//------------------------------------------------------------------------------
TCPServerT TCPServerWIZspi;

TCPServerWIZspiAdapterT WIZspiAdapter =
{
	//.SPI = (REG_SPI_T*)SPI2
};
//==============================================================================
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
static void EventListener(TCPServerT* server, TCPServerEventSelector selector, uint32_t args, uint32_t count)
{
	switch ((uint8_t)selector)
	{
		case TCPServerEventEndLine :
			//xRxDeclareRequest(&serial_port->Rx, xRxRequestClearBuffer, 0, 0);
			RequestsReceiver(&server->Rx, (uint8_t*)args, count);
			break;
		
		case TCPServerEventReceiverBufferIsFull :
			//xRxDeclareRequest(&serial_port->Rx, xRxRequestClearBuffer, 0, 0);
			RequestsReceiver(&server->Rx, (uint8_t*)args, count);
			break;
	}
}
//------------------------------------------------------------------------------
static xResult RequestListener(TCPServerT* server, TCPServerRequestSelector selector, uint32_t args, uint32_t count)
{
	switch ((uint8_t)selector)
	{
		case TCPServerRequestDelay :
			HAL_Delay(args);
			break;
		
		default : return xResultRequestIsNotFound;
	}
	
	return xResultAccept;
}
//==============================================================================
TCPServerInterfaceT Interface =
{
	.EventListener = (TCPServerEventListenerT)EventListener,
	.RequestListener = (TCPServerRequestListenerT)RequestListener
};
//==============================================================================
/**
 * @brief main handler
 */
inline void TCPServerWIZspiComponentHandler()
{

}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
inline void TCPServerWIZspiComponentTimeSynchronization()
{

}
//==============================================================================
xResult TCPServerWIZspiComponentInit(void* parent)
{
	extern xDataBufferT MainResponseBuffer;
	WIZspiAdapter.ResponseBuffer = &MainResponseBuffer;
	
	WIZspiAdapter.Server = &TCPServerWIZspi;
	
	//WIZspi = (REG_SPI_T*)SPI4;
	
	WIZspiAdapter.BusInterface.HardwareResetOn = WIZspiHardwareResetOn;
	WIZspiAdapter.BusInterface.HardwareResetOff = WIZspiHardwareResetOff;
	WIZspiAdapter.BusInterface.SelectChip = WIZspiSelectChip;
	WIZspiAdapter.BusInterface.DeselectChip = WIZspiDeselectChip;
	WIZspiAdapter.BusInterface.ReceiveByte = WIZspiReceiveByte;
	WIZspiAdapter.BusInterface.TransmiteByte = WIZspiTransmiteByte;
	
	WIZspiAdapter.RxBuffer = rx_buf;
	WIZspiAdapter.RxBufferSize = sizeof(rx_buf);
	
	xRxReceiverInit(&WIZspiAdapter.RxReceiver,
									&TCPServerWIZspi.Rx,
									0,
									rx_receiver_buf,
									sizeof(rx_receiver_buf));
	
	TCPServerInit(&TCPServerWIZspi, parent, &Interface);
	TCPServerWIZspiAdapterInit(&TCPServerWIZspi, &WIZspiAdapter);
	
  return xResultAccept;
}
//==============================================================================
#endif /* TCP_SERVER_COMPONENT_ENABLE */
