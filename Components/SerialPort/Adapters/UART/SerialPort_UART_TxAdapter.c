//==============================================================================
//module enable:

#include "SerialPort/Adapters/SerialPort_AdapterConfig.h"
#ifdef SERIAL_PORT_UART_ADAPTER_ENABLE
//==============================================================================
//includes:

#include "SerialPort_UART_TxAdapter.h"
//==============================================================================
//functions:

static void PrivateHandler(xTxT *tx)
{
	SerialPortUART_AdapterT* adapter = tx->Adapter;
	
	if (!adapter->Usart->Control1.TxEmptyInterruptEnable
	&& adapter->TxCircleBuffer.TotalIndex != adapter->TxCircleBuffer.HandlerIndex)
	{
		adapter->Usart->Control1.TxEmptyInterruptEnable = true;
	}
	
	tx->Status.Transmitter = (adapter->Usart->Control1.TxEmptyInterruptEnable > 0)
							? xTxStatusIsTransmits : xTxStatusIdle;
}
//------------------------------------------------------------------------------

static void PrivateEventListener(xTxT *tx, xTxEventSelector event, void* arg, ...)
{
	//SerialPortUART_AdapterT* adapter = tx->Adapter;
	
	switch ((int)event)
	{
		default : break;
	}
}
//------------------------------------------------------------------------------

static void PrivateIRQListener(xTxT *tx)
{
	SerialPortUART_AdapterT* adapter = tx->Adapter;

	if (adapter->Usart->Control1.TxEmptyInterruptEnable && adapter->Usart->Status.TxEmpty)
	{
		if (adapter->TxCircleBuffer.HandlerIndex != adapter->TxCircleBuffer.TotalIndex)
		{
			adapter->Usart->Data = xCircleBufferGet(&adapter->TxCircleBuffer);
		}
		else
		{
			adapter->Usart->Control1.TxEmptyInterruptEnable = false;
		}
	}
}

//------------------------------------------------------------------------------

static xResult PrivateRequestListener(xTxT* tx, xTxRequestSelector selector, void* arg, ...)
{
	SerialPortUART_AdapterT* adapter = tx->Adapter;

	switch ((uint32_t)selector)
	{
		case xTxRequestEnableTransmitter:
			adapter->Usart->Control1.TxEmptyInterruptEnable = true;
			break;

		case xTxRequestUpdateTransmitterStatus:
			tx->Status.Transmitter = adapter->Usart->Control1.TxEmptyInterruptEnable
			? xTxStatusIsTransmits : xTxStatusIdle;
			break;

		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------

static xResult PrivateTransmitData(xTxT* tx, void* data, uint32_t size)
{
	SerialPortUART_AdapterT* adapter = tx->Adapter;

	if (xCircleBufferGetFreeSize(&adapter->TxCircleBuffer) >= size)
	{
		xCircleBufferAdd(&adapter->TxCircleBuffer, (uint8_t*)data, size);
		return xResultAccept;
	}
	return xResultError;
}
//------------------------------------------------------------------------------

static uint32_t PrivateGetBufferSize(xTxT* tx)
{
	SerialPortUART_AdapterT* adapter = tx->Adapter;

	return adapter->TxCircleBuffer.SizeMask + 1;
}
//------------------------------------------------------------------------------

static uint32_t PrivateGetFreeBufferSize(xTxT* tx)
{
	SerialPortUART_AdapterT* adapter = tx->Adapter;

	return xCircleBufferGetFreeSize(&adapter->TxCircleBuffer);
}
//==============================================================================
//initialization:

static xTxInterfaceT PrivateSerialPortUART_TxAdapterInterface =
{
	INITIALIZATION_HANDLER(xTx, PrivateHandler),
	INITIALIZATION_EVENT_LISTENER(xTx, PrivateEventListener),
	INITIALIZATION_IRQ_LISTENER(xTx, PrivateIRQListener),
	INITIALIZATION_REQUEST_LISTENER(xTx, PrivateRequestListener),

	.TransmitData = (xTxTransmitDataT)PrivateTransmitData,

	.GetBufferSize = (xTxGetBufferSizeActionT)PrivateGetBufferSize,
	.GetFreeBufferSize = (xTxGetFreeBufferSizeActionT)PrivateGetFreeBufferSize
};
//------------------------------------------------------------------------------

xResult SerialPortUART_TxAdapterInit(SerialPortT* serial_port, SerialPortUART_AdapterT* adapter)
{
	if (serial_port && adapter)
	{
		adapter->Usart->Control1.TransmitterEnable = true;
		adapter->Usart->Control1.TxCompleteInterruptEnable = false;
		adapter->Usart->Control1.TxEmptyInterruptEnable = false;
		
		return xTxInit(&serial_port->Tx, serial_port, adapter, &PrivateSerialPortUART_TxAdapterInterface);
	}
	
	return xResultError;
}
//==============================================================================
#endif //SERIAL_PORT_UART_ADAPTER_ENABLE
