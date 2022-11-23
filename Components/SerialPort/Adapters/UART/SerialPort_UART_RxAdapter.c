//==============================================================================
//module enable:

#include "SerialPort/Adapters/SerialPort_AdapterConfig.h"
#ifdef SERIAL_PORT_UART_ADAPTER_ENABLE
//==============================================================================
//includes:

#include "SerialPort_UART_RxAdapter.h"
#include "SerialPort/Controls/SerialPort.h"
//==============================================================================
static void PrivateHandler(xRxT* rx)
{
	SerialPortUART_AdapterT* adapter = rx->Adapter;
	
	adapter->RxCircleBuffer.TotalIndex = (adapter->RxCircleBuffer.SizeMask + 1) -
											((DMA_Stream_TypeDef*)adapter->RxDMA->Instance)->NDTR;
	
	xRxReceiverRead(&adapter->RxReceiver, &adapter->RxCircleBuffer);
}
//------------------------------------------------------------------------------
static void PrivateIRQListener(xRxT* rx)
{

}
//------------------------------------------------------------------------------
static void PrivateEventListener(xRxT* rx, xRxEventSelector event, void* arg, ...)
{
	switch ((uint32_t)event)
	{
		default : break;
	}
}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(xRxT* rx, xRxRequestSelector selector, void* arg, ...)
{
	SerialPortUART_AdapterT* adapter = rx->Adapter;
	
	switch ((uint32_t)selector)
	{
		case xRxRequestClearBuffer:
			adapter->RxReceiver.BytesReceived = 0;
			break;

		case xRxRequestClearResponseBuffer:
			xDataBufferClear(adapter->ResponseBuffer);
			break;
		
		default : return xResultRequestIsNotFound;
	}
	
	return xResultAccept;
}
//------------------------------------------------------------------------------

static xResult PrivateReceive(xRxT* rx, uint8_t* data, uint32_t size)
{
	SerialPortUART_AdapterT* adapter = rx->Adapter;
	
	xRxReceiverReceive(&adapter->RxReceiver, data, size);

	return xResultAccept;
}
//------------------------------------------------------------------------------

static xResult PrivatePutInResponseBuffer(xRxT* rx, uint8_t* data, uint32_t size)
{
	SerialPortUART_AdapterT* adapter = rx->Adapter;

	xDataBufferAdd(adapter->ResponseBuffer, data, size);

	return xResultAccept;
}
//------------------------------------------------------------------------------

static uint8_t* PrivateGetBuffer(xRxT* rx)
{
	SerialPortUART_AdapterT* adapter = rx->Adapter;

	return adapter->RxReceiver.Buffer;
}
//------------------------------------------------------------------------------

static uint32_t PrivateGetBufferSize(xRxT* rx)
{
	SerialPortUART_AdapterT* adapter = rx->Adapter;

	return adapter->RxReceiver.BufferSize;
}
//------------------------------------------------------------------------------

static uint32_t PrivateGetBytesCountInBuffer(xRxT* rx)
{
	SerialPortUART_AdapterT* adapter = rx->Adapter;

	return adapter->RxReceiver.BytesReceived;
}
//------------------------------------------------------------------------------

static uint8_t* PrivateGetResponseBuffer(xRxT* rx)
{
	SerialPortUART_AdapterT* adapter = rx->Adapter;

	return adapter->ResponseBuffer->Data;
}
//------------------------------------------------------------------------------

static uint32_t PrivateGetResponseBufferSize(xRxT* rx)
{
	SerialPortUART_AdapterT* adapter = rx->Adapter;

	return adapter->ResponseBuffer->Size;
}
//------------------------------------------------------------------------------

static uint32_t PrivateGetBytesCountInResponseBuffer(xRxT* rx)
{
	SerialPortUART_AdapterT* adapter = rx->Adapter;

	return adapter->ResponseBuffer->DataSize;
}
//------------------------------------------------------------------------------

static void PrivateRxReceiverEventListener(xRxReceiverT* receiver, xRxReceiverEventSelector event, void* arg, ...)
{
	SerialPortT* serial_port = receiver->Parent->Object.Parent;
	
	SerialPortReceivedDataT received_data =
	{
		.Data = arg,
		.Size = *(uint32_t*)(&arg + 1),
	};

	switch ((uint8_t)event)
	{
		case xRxReceiverEventEndLine:
			SerialPortEventListener(serial_port, SerialPortEventEndLine, &received_data);
			break;
		
		case xRxReceiverEventBufferIsFull:
			SerialPortEventListener(serial_port, SerialPortEventBufferIsFull, &received_data);
			break;

		default: return;
	}
}
//==============================================================================
//interface initializations:

static const xRxInterfaceT rx_interface =
{
	INITIALIZATION_HANDLER(xRx, PrivateHandler),

	INITIALIZATION_IRQ_LISTENER(xRx, PrivateIRQListener),

	INITIALIZATION_EVENT_LISTENER(xRx, PrivateEventListener),
	INITIALIZATION_REQUEST_LISTENER(xRx, PrivateRequestListener),

	.Receive = (xRxReceiveActionT)PrivateReceive,
	.PutInResponseBuffer = (xRxPutInResponseBufferActionT)PrivatePutInResponseBuffer,

	.GetBuffer = (xRxGetBufferActionT)PrivateGetBuffer,
	.GetBufferSize = (xRxGetBufferSizeActionT)PrivateGetBufferSize,
	.GetBytesCountInBuffer = (xRxGetBytesCountInBufferActionT)PrivateGetBytesCountInBuffer,

	.GetResponseBuffer = (xRxGetResponseBufferActionT)PrivateGetResponseBuffer,
	.GetResponseBufferSize = (xRxGetResponseBufferSizeActionT)PrivateGetResponseBufferSize,
	.GetBytesCountInResponseBuffer = (xRxGetBytesCountInResponseBufferActionT)PrivateGetBytesCountInResponseBuffer
};
//------------------------------------------------------------------------------

static xRxReceiverInterfaceT rx_receiver_interface =
{
	INITIALIZATION_EVENT_LISTENER(xRxReceiver, PrivateRxReceiverEventListener),
};
//==============================================================================
//initializations:

xResult SerialPortUART_RxAdapterInit(SerialPortT* serial_port, SerialPortUART_AdapterT* adapter)
{
	if (serial_port && adapter)
	{
		adapter->RxReceiver.Interface = &rx_receiver_interface;
		
		if (xRxInit(&serial_port->Rx, serial_port, adapter, &rx_interface) != xResultAccept)
		{
			return xResultError;
		}
		
		adapter->Usart->Control1.ReceiverEnable = true;
		adapter->Usart->Control3.DMA_EnableReceiver = true;
		
		if (adapter->RxDMA)
		{
			uint8_t dma_result = HAL_DMA_Start(adapter->RxDMA,
												(uint32_t)&adapter->Usart->Data,
												(uint32_t)adapter->RxCircleBuffer.Buffer,
												adapter->RxCircleBuffer.SizeMask + 1);
			
			if (dma_result != HAL_OK)
			{
				return xResultError;
			}
		}
		
		return xResultAccept;
	}
	
	return xResultError;
}
//==============================================================================
#endif //SERIAL_PORT_UART_ADAPTER_ENABLE
