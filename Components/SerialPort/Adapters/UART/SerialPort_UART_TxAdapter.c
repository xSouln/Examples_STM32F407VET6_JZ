//==============================================================================
//module enable:

#include "SerialPort/Adapters/SerialPort_AdapterConfig.h"
#ifdef SERIAL_PORT_UART_ADAPTER_ENABLE
//==============================================================================
//includes:

#include "SerialPort_UART_TxAdapter.h"
//==============================================================================
static void Handler(xTxT *tx)
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
static void EventListener(xTxT *tx, xTxEventSelector event, uint32_t args, uint32_t count)
{
	SerialPortUART_AdapterT* adapter = tx->Adapter;
	
	switch ((uint32_t)event)
	{
		case xTxEventIRQ :
		{
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
		break;
		
		default : break;
	}
}
//------------------------------------------------------------------------------
static xResult RequestListener(xTxT* tx, xTxRequestSelector selector, uint32_t args, uint32_t count)
{
	SerialPortUART_AdapterT* adapter = tx->Adapter;
	
	switch ((uint32_t)selector)
	{
		case xTxRequestTransmitData :
			if (xCircleBufferGetFreeSize(&adapter->TxCircleBuffer) >= count)
			{
				xCircleBufferAdd(&adapter->TxCircleBuffer, (uint8_t*)args, count);
				return xResultAccept;
			}
			return xResultError;
		
		case xTxRequestEnableTransmitter :
			adapter->Usart->Control1.TxEmptyInterruptEnable = true;
			break;
			
			
		default : return xResultRequestIsNotFound;
	}
	
	return xResultRequestIsNotFound;
}
//------------------------------------------------------------------------------
static int GetValue(xTxT* tx, xTxValueSelector selector)
{
	SerialPortUART_AdapterT* adapter = tx->Adapter;
	
	switch ((uint32_t)selector)
	{
		case xTxValueTransmitterStatus:
			tx->Status.Transmitter = adapter->Usart->Control1.TxEmptyInterruptEnable ? xTxStatusIsTransmits : xTxStatusIdle;
			return tx->Status.Transmitter;
		
		case xTxValueBufferSize:
			return (adapter->TxCircleBuffer.SizeMask + 1);
		
		case xTxValueFreeBufferSize:
			return xCircleBufferGetFreeSize(&adapter->TxCircleBuffer);
		
		default : return 0;
	}
}
//------------------------------------------------------------------------------
static xResult SetValue(xTxT* tx, xTxValueSelector selector, uint32_t value)
{
	//SerialPortUART_AdapterT* adapter = tx->Adapter;
	
	switch ((uint32_t)selector)
	{
		default : return xResultValueIsNotFound;
	}
	
	return xResultValueIsNotFound;
}
//------------------------------------------------------------------------------
static xTxInterfaceT interface =
{
	.Handler = (xTxHandlerT)Handler,
	.EventListener = (xTxEventListenerT)EventListener,
	.RequestListener = (xTxRequestListenerT)RequestListener,
	.GetValue = (xTxActionGetValueT)GetValue,
	.SetValue = (xTxActionSetValueT)SetValue
};
//==============================================================================
xResult SerialPortUART_TxAdapterInit(SerialPortT* serial_port, SerialPortUART_AdapterT* adapter)
{
	if (serial_port && adapter)
	{
		adapter->Usart->Control1.TransmitterEnable = true;
		adapter->Usart->Control1.TxCompleteInterruptEnable = false;
		adapter->Usart->Control1.TxEmptyInterruptEnable = false;
		
		return xTxInit(&serial_port->Tx, serial_port, adapter, &interface);
	}
	
	return xResultError;
}
//==============================================================================
#endif //SERIAL_PORT_UART_ADAPTER_ENABLE
