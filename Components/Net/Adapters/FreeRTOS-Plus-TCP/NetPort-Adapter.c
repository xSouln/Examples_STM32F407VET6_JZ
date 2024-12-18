//==============================================================================
//header:

#include "NetPort-Adapter.h"

#ifdef _NET_PORT_ADAPTER_H_
//==============================================================================
//includes:

#include "Abstractions/xReflection/xReflection.h"
//==============================================================================
//types:

typedef struct
{
	xPropertyProviderHandleT Base;

	xPortT* Port;
	xNetT* Net;

} PropertyProviderHandleT;
//==============================================================================
//variables:

static const xPropertyDescriptionT privatePropertiesInfo[] =
{
	{ .PropertyId = xNetOptionsFlagsProperty, .Info = xPropertyFlagsType, .SizeInfo = xPropertyTypeSizeHalfWord },
	{ .PropertyId = xNetOptionsServerPortProperty, .Info = xPropertyBaseType, .SizeInfo = xPropertyTypeSizeHalfWord },
	{ .PropertyId = xNetOptionsAddressProperty, .Info = xPropertyCustomType, .SizeInfo = xPropertyTypeSizeWord },
	{ .PropertyId = xNetOptionsNetMaskProperty, .Info = xPropertyCustomType, .SizeInfo = xPropertyTypeSizeWord },
	{ .PropertyId = xNetOptionsGatewayAddressProperty, .Info = xPropertyCustomType, .SizeInfo = xPropertyTypeSizeWord },
	{ .PropertyId = xNetOptionsDNSServerAddressProperty, .Info = xPropertyCustomType, .SizeInfo = xPropertyTypeSizeWord },
	{ .PropertyId = xNetOptionsSNTPServerProperty, .Info = xPropertyStringType, .SizeInfo = xPropertyTypeSizeByte },
};
//==============================================================================
//functions:

static void privateRxTask(xPortT* port)
{
	NetPortAdapterT* adapter = (NetPortAdapterT*)port->Adapter.Content;
	xNetSocketT* socket = port->Binding;

	while (true)
	{
		if (socket != NULL && socket->State == xNetSocketEstablished)
		{
			int len = xNetReceive(socket, adapter->RxOperationBuffer, adapter->RxOperationBufferSize);

			if (len > 0)
			{
				xCircleBufferAdd(&adapter->Internal.RxCircleBuffer, adapter->RxOperationBuffer, len);
			}
		}
	}
}
//------------------------------------------------------------------------------

static void PrivateHandler(xPortT* port)
{
	NetPortAdapterT* adapter = (NetPortAdapterT*)port->Adapter.Content;
	xNetSocketT* socket = port->Binding;

	xNetSocketHandler(socket);

	xRxReceiverRead(&adapter->RxReceiver, &adapter->Internal.RxCircleBuffer);

	if (socket != NULL && socket->State == xNetSocketEstablished)
	{
		if (adapter->TxBuffer.Length)
		{
			xSemaphoreTake(adapter->TransactionMutex, portMAX_DELAY);
			xNetTransmit(socket, adapter->TxBuffer.Data, adapter->TxBuffer.Length);
			adapter->TxBuffer.Length = 0;
			xSemaphoreGive(adapter->TransactionMutex);
		}
	}
}
//------------------------------------------------------------------------------

static xResult privateOptionsGetter(PropertyProviderHandleT* handle, xPropertyDescriptionT descriptions)
{
	switch (descriptions.PropertyId)
	{
		case xNetOptionsFlagsProperty:
		{
			xNetOptionsFlagsT flags;

			flags.IsEnabled = true;
			flags.ServerIsEnabled = true;
			flags.DHCP_IsEnabled = !handle->Net->StaticAddressIsEnabled;
			flags.SNTP_IsEnabled = handle->Net->SNTP_IsEnabled;

			xDataBufferAdd(handle->Base.Out, &flags, sizeof(flags));
			break;
		}

		case xNetOptionsServerPortProperty:
			xDataBufferAdd(handle->Base.Out, &handle->Net->ServerPort, sizeof(handle->Net->ServerPort));
			break;

		case xNetOptionsAddressProperty:
			xDataBufferAdd(handle->Base.Out, &handle->Net->LocalAddress.Value, sizeof(handle->Net->LocalAddress.Value));
			break;

		case xNetOptionsNetMaskProperty:
			xDataBufferAdd(handle->Base.Out, &handle->Net->NetMask.Value, sizeof(handle->Net->NetMask.Value));
			break;

		case xNetOptionsGatewayAddressProperty:
			xDataBufferAdd(handle->Base.Out, &handle->Net->GatewayAddress.Value, sizeof(handle->Net->GatewayAddress.Value));
			break;

		case xNetOptionsDNSServerAddressProperty:
			xDataBufferAdd(handle->Base.Out, &handle->Net->DNSServerAddress.Value, sizeof(handle->Net->DNSServerAddress.Value));
			break;

		case xNetOptionsSNTPServerProperty:
			xDataBufferAdd(handle->Base.Out, SNTP_SERVER, sizeof(SNTP_SERVER));
			break;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------

static xResult privateOptionsSetter(PropertyProviderHandleT* handle,
		xPropertyDescriptionT descriptions,
		xMemoryReaderT* memoryReader)
{
	switch (descriptions.PropertyId)
	{
		case xNetOptionsFlagsProperty:
		{
			xNetOptionsFlagsT flags = { 0 };
			xResult result = xMemoryReaderRead(memoryReader, &flags, sizeof(flags));

			if (result == xResultAccept)
			{
				handle->Net->StaticAddressIsEnabled = !flags.DHCP_IsEnabled;
				handle->Net->SNTP_IsEnabled = flags.SNTP_IsEnabled;
			}

			return result;
		}

		case xNetOptionsServerPortProperty:
			return xMemoryReaderRead(memoryReader, &handle->Net->ServerPort, sizeof(handle->Net->ServerPort));

		case xNetOptionsAddressProperty:
			return xMemoryReaderRead(memoryReader, &handle->Net->LocalAddress.Value, sizeof(handle->Net->LocalAddress.Value));

		case xNetOptionsNetMaskProperty:
			return xMemoryReaderRead(memoryReader, &handle->Net->NetMask.Value, sizeof(handle->Net->NetMask.Value));

		case xNetOptionsGatewayAddressProperty:
			return xMemoryReaderRead(memoryReader, &handle->Net->GatewayAddress.Value, sizeof(handle->Net->GatewayAddress.Value));

		case xNetOptionsDNSServerAddressProperty:
			return xMemoryReaderRead(memoryReader, &handle->Net->DNSServerAddress.Value, sizeof(handle->Net->DNSServerAddress.Value));

		case xNetOptionsSNTPServerProperty:
			xMemoryReaderOffsetString(memoryReader);
			//xDataBufferAdd(handle->Base.Out, SNTP_SERVER, sizeof(SNTP_SERVER));
			return xResultNotSupported;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------

static xResult privateProvideFunction(xPortT* port, RequestListenerInArgT* in, RequestListenerOutArgT* out)
{
	xFunctionProviderArgT* request = in->Content;

	switch (request->Action)
	{
		case NetPortProviderSaveOptions:
			return MqttSaveObject(port, 0);
	}

	return xResultNotSupported;
}
//------------------------------------------------------------------------------

static xResult PrivateRequestListener(xPortT* port,
		xPortAdapterRequestSelector selector,
		uint32_t description,
		void* arg,
		void* out)
{
	NetPortAdapterT* adapter = (NetPortAdapterT*)port->Adapter.Content;
	xNetSocketT* socket = port->Binding;

	switch ((uint32_t)selector)
	{
		case xPortAdapterRequestUpdateTxStatus:
			port->Tx.IsEnable = socket != 0 && (int)socket->Handle != -1;
			break;

		case xPortAdapterRequestUpdateRxStatus:
			port->Rx.IsEnable = socket != 0 && (int)socket->Handle != -1;
			break;

		case xPortAdapterRequestGetRxBuffer:
			*(uint8_t**)arg = adapter->RxReceiver.Buffer;
			break;

		case xPortAdapterRequestGetRxBufferSize:
			*(uint32_t*)arg = adapter->RxReceiver.BufferSize;
			break;

		case xPortAdapterRequestGetRxBufferFreeSize:
			*(uint32_t*)arg = adapter->RxReceiver.BufferSize - adapter->RxReceiver.BytesReceived;
			break;

		case xPortAdapterRequestClearRxBuffer:
			adapter->RxReceiver.BytesReceived = 0;
			break;

		case xPortAdapterRequestGetTxBufferSize:
			*(uint32_t*)arg = socket != 0 && (int)socket->Handle != -1 ? 1000 : 0;
			break;

		case xPortAdapterRequestGetTxBufferFreeSize:
			*(uint32_t*)arg = socket != 0 && (int)socket->Handle != -1 ? 1000 : 0;
			break;

		case xPortAdapterRequestSetBinding:
			port->Binding = arg;
			break;

		case xPortAdapterRequestStartTransmission:
			xSemaphoreTake(adapter->TransactionMutex, portMAX_DELAY);
			break;

		case xPortAdapterRequestEndTransmission:
		{
			if (socket->State == xNetSocketEstablished)
			{
				xNetTransmit(socket, adapter->TxBuffer.Data, adapter->TxBuffer.Length);
				adapter->TxBuffer.Length = 0;
			}
			xSemaphoreGive(adapter->TransactionMutex);
			break;
		}

		case xPortRequestGetOptions:
		{
			xPropertyProviderArgT* request = arg;

			if (request == null || request->SizeOfParameters == 0)
			{
				return xResultError;
			}

			PropertyProviderHandleT providerHandle;
			providerHandle.Base.Provider = (void*)privateOptionsGetter;
			providerHandle.Base.Out = request->Out;
			providerHandle.Net = adapter->Net;
			providerHandle.Port = port;

			xReflectionGetterProvider(&providerHandle.Base,
					request->Info,
					request->SizeOfParameters,
					privatePropertiesInfo,
					sizeof_array(privatePropertiesInfo));

			break;
		}

		case xPortRequestSetOptions:
		{
			xPropertyProviderArgT* request = arg;

			if (request == null || request->SizeOfParameters == 0)
			{
				return xResultError;
			}

			PropertyProviderHandleT providerHandle;
			providerHandle.Base.Provider = (void*)privateOptionsSetter;
			providerHandle.Base.Out = request->Out;
			providerHandle.Net = adapter->Net;
			providerHandle.Port = port;

			xReflectionSetterProvider(&providerHandle.Base,
					request->Info,
					request->SizeOfParameters,
					privatePropertiesInfo,
					sizeof_array(privatePropertiesInfo));

			break;
		}

		case xPortRequestProvideFunction:
			{
				return privateProvideFunction(port, arg, out);
			}

		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static void PrivateEventListener(xPortT* port, xPortAdapterEventSelector selector, uint32_t description, void* arg)
{
	//register UsartPortAdapterT* adapter = (UsartPortAdapterT*)port->Adapter;

	switch((int)selector)
	{
		default: return;
	}
}
//------------------------------------------------------------------------------
static int PrivateTransmit(xPortT* port, void* data, uint32_t size)
{
	NetPortAdapterT* adapter = (NetPortAdapterT*)port->Adapter.Content;
	//xNetSocketT* socket = port->Binding;

	xDataBufferAdd(&adapter->TxBuffer, data, size);

	//return xNetTransmit(socket, data, size);
	return size;
}
//------------------------------------------------------------------------------
static int PrivateReceive(xPortT* port, void* data, uint32_t size)
{
	xNetSocketT* socket = port->Binding;

	return xNetReceive(socket, data, size);
}
//------------------------------------------------------------------------------
static void PrivateRxReceiverEventListener(xRxReceiverT* receiver, xRxReceiverEventSelector event, void* arg)
{
	register xPortT* port = receiver->Base.Parent;

	switch ((uint8_t)event)
	{
		case xRxReceiverEventEndLine:
			xPortEventListener(port, xPortEventRxFoundEndLine, 0, arg);
			break;

		case xRxReceiverEventBufferIsFull:
			xPortEventListener(port, xPortEventRxBufferIsFull, 0, arg);
			break;

		default: return;
	}
}
//==============================================================================
//initializations:

static xPortAdapterInterfaceT privatePortInterface =
{
	.Handler = (xPortAdapterHandlerT)PrivateHandler,

	.RequestListener = (xPortAdapterRequestListenerT)PrivateRequestListener,
	.EventListener = (xPortAdapterEventListenerT)PrivateEventListener,

	.Transmit = (xPortAdapterTransmitActionT)PrivateTransmit,
	.Receive = (xPortAdapterReceiveActionT)PrivateReceive
};
//------------------------------------------------------------------------------

xResult NetPortAdapterInit(xPortT* port, NetPortAdapterT* adapter, NetPortAdapterInitT* adapterInit)
{
	if (port)
	{
		port->Adapter.Description = nameof(UsartPortAdapterT);
		port->Adapter.Content = adapter;
		port->Adapter.Interface = &privatePortInterface;

		port->InterfaceType = xPortInterfaceEthernet;
		port->InterfaceMode = xPortTransferLayerByteStream;

		adapter->Net = adapterInit->Net;

		adapter->RxOperationBuffer = adapterInit->RxOperationBuffer;
		adapter->RxOperationBufferSize = adapterInit->RxOperationBufferSize;

		adapter->Internal.RxCircleBuffer.SizeMask = 0x1ff;
		adapter->Internal.RxCircleBuffer.Memory = adapter->Internal.RxCircleBufferMemory;

		adapter->Net->SNTP_IsEnabled = true;

		xRxReceiverInit(&adapter->RxReceiver, 
						port,
						PrivateRxReceiverEventListener,
						adapterInit->RxBuffer,
						adapterInit->RxBufferSize);

		xDataBufferInit(&adapter->TxBuffer,
						adapterInit->TxBuffer,
						adapterInit->TxBufferSize);

		adapter->TransactionMutex = xSemaphoreCreateMutex();
		
		xTaskCreate((void*)privateRxTask, // Function that implements the task.
						"net port task", // Text name for the task.
						0x100, // Number of indexes in the xStack array.
						port, // Parameter passed into the task.
						osPriorityNormal, // Priority at which the task is created.
						&adapter->Internal.RxTaskHandle);

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
#endif //_NET_PORT_ADAPTER_H_
