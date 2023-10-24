//==============================================================================
//includes:

#include "CAN_LocalPort-Adapter.h"
//==============================================================================
//defines:


//==============================================================================
//types:

typedef struct
{
	uint32_t Identifier;
	uint8_t DataLength;

	union
	{
		struct
		{
			uint32_t DataL;
			uint32_t DataH;
		};

		uint8_t Data[8];
	};

} CAN_PacketT;
//==============================================================================
//variables:


//==============================================================================
//functions:

static void PrivateHandler(xPortT* port)
{
	CAN_LocalPortAdapterT* adapter = (CAN_LocalPortAdapterT*)port->Adapter.Content;

	if (adapter->CAN_Register->TxStatus.Mailbox0_TransmissionError)
	{
		adapter->CAN_Register->TxStatus.Mailbox0_AbortRequest = true;
	}

	if (adapter->CAN_Register->TxStatus.Mailbox1_TransmissionError)
	{
		adapter->CAN_Register->TxStatus.Mailbox1_AbortRequest = true;
	}

	if (adapter->CAN_Register->TxStatus.Mailbox2_TransmissionError)
	{
		adapter->CAN_Register->TxStatus.Mailbox2_AbortRequest = true;
	}

	if (!adapter->AwaitTxValidation)
	{
		if (adapter->TxValidationComplite)
		{
			adapter->TxValidationComplite = false;

			if (!adapter->TxError)
			{
				xCircleBufferOffsetHandlerIndex(&adapter->TxCircleBuffer, adapter->TxHeader.DLC);
			}
		}

		uint8_t data[8];
		int dataSize = xCircleBufferTryRead(&adapter->TxCircleBuffer, data, sizeof(data));

		if (dataSize)
		{
			uint32_t txMailbox;

			adapter->TxHeader.DLC = dataSize;

			adapter->TxError = 0;
			adapter->AwaitTxValidation = true;
			adapter->TxValidationComplite = false;

			if (HAL_CAN_AddTxMessage(adapter->CAN, &adapter->TxHeader, data, &txMailbox) != HAL_OK)
			{
				adapter->AwaitTxValidation = false;
			}
		}
	}
}
//------------------------------------------------------------------------------
static void PrivateIRQ(xCAN_T* can, xCAN_HandleT* reg, CAN_LocalPortAdapterT* adapter)
{

}
//------------------------------------------------------------------------------
static void PrivateRxIRQ(xCAN_T* can, xCAN_HandleT* reg, CAN_LocalPortAdapterT* adapter)
{
	if (reg->RxFIFO0.Overrun)
	{
		reg->RxFIFO0.Overrun = false;
	}

	if (reg->RxFIFO0.Full)
	{
		reg->RxFIFO0.Full = false;
	}

	if (reg->RxFIFO0.MessagePending)
	{
		CAN_PacketT packet;

		packet.Identifier = reg->RxMailBox[0].Identifier.StandardIdentifier;
		packet.DataLength = reg->RxMailBox[0].DataControlAndTimeStamp.DataLengthCode;
		packet.DataL = reg->RxMailBox[0].DataLow.Value;
		packet.DataH = reg->RxMailBox[0].DataHigh.Value;

		xCircleBufferAdd(&adapter->RxCircleBuffer, packet.Data, packet.DataLength);

		reg->RxFIFO0.ReleaseOutputMailbox = true;
	}
}
//------------------------------------------------------------------------------
static void PrivateTxIRQ(xCAN_T* can, xCAN_HandleT* reg, CAN_LocalPortAdapterT* adapter)
{
	if (reg->TxStatus.Mailbox0_RequestCompleted)
	{
		reg->TxStatus.Mailbox0_TransmissionAccepted = false;
		adapter->TxError |= reg->TxStatus.Mailbox0_TransmissionError;
	}

	if (reg->TxStatus.Mailbox1_RequestCompleted)
	{
		reg->TxStatus.Mailbox1_TransmissionAccepted = false;
		adapter->TxError |= reg->TxStatus.Mailbox1_TransmissionError;
	}

	if (reg->TxStatus.Mailbox2_RequestCompleted)
	{
		reg->TxStatus.Mailbox2_TransmissionAccepted = false;
		adapter->TxError |= reg->TxStatus.Mailbox2_TransmissionError;
	}

	adapter->TxValidationComplite = true;
	adapter->AwaitTxValidation = false;
}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(xPortT* port, xPortAdapterRequestSelector selector, void* arg)
{
	CAN_LocalPortAdapterT* adapter = (CAN_LocalPortAdapterT*)port->Adapter.Content;

	switch ((uint32_t)selector)
		{
			case xPortAdapterRequestGetTxBufferSize:
				*(uint32_t*)arg = adapter->TxCircleBuffer.SizeMask + 1;
				break;

			case xPortAdapterRequestGetTxBufferFreeSize:
				*(uint32_t*)arg = xCircleBufferGetFreeSize(&adapter->TxCircleBuffer);
				break;

			case xPortAdapterRequestClearTxBuffer:
				adapter->TxCircleBuffer.HandlerIndex = adapter->TxCircleBuffer.TotalIndex;
				break;

			case xPortAdapterRequestSetBinding:
				port->Binding = arg;
				break;

			case xPortAdapterRequestStartTransmission:
	#ifdef INC_FREERTOS_H
				xSemaphoreTake(adapter->TransactionMutex, portMAX_DELAY);
	#endif
				break;

			case xPortAdapterRequestEndTransmission:
	#ifdef INC_FREERTOS_H
				xSemaphoreGive(adapter->TransactionMutex);
	#endif
				break;

			default : return xResultRequestIsNotFound;
		}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static void PrivateEventListener(xPortT* port, xPortAdapterEventSelector selector, void* arg)
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
	CAN_LocalPortAdapterT* adapter = (CAN_LocalPortAdapterT*)port->Adapter.Content;

	uint8_t* in = data;
	int result = size;

	while (size)
	{
		uint16_t packetSize = xCircleBufferGetFreeSize(&adapter->TxCircleBuffer);

		if (packetSize > size)
		{
			packetSize = size;
		}

		if (packetSize)
		{
			xCircleBufferAdd(&adapter->TxCircleBuffer, in, packetSize);

			in += packetSize;
			size -= packetSize;
		}
	}

	return result;
}
//------------------------------------------------------------------------------
static int PrivateReceive(xPortT* port, void* data, uint32_t size)
{
	return -xResultNotSupported;
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
xResult CAN_LocalPortAdapterInit(xPortT* port, struct xPortAdapterInitT* init)
{
	if (port && init)
	{
		CAN_LocalPortAdapterT* adapter = init->Adapter;
		CAN_LocalPortAdapterInitT* adapterInit = init->Init;

		port->Adapter.Description = nameof(CAN_LocalPortAdapterT);
		port->Adapter.Content = adapter;
		port->Adapter.Interface = &privatePortInterface;

		adapter->CAN = adapterInit->CAN;
		adapter->CAN_Register = (void*)adapterInit->CAN->Instance;

		adapter->CAN_Register->Interrupts.TransmitMailboxEmptyInterruptEnable = true;

		xCAN_CoreBind(adapterInit->CAN_Number, adapter->CAN_Register, port);
		xCAN_RegisterIRQ(adapterInit->CAN_Number, (void*)PrivateIRQ, adapter);
		xCAN_RegisterRxIRQ(adapterInit->CAN_Number, (void*)PrivateRxIRQ, adapter);
		xCAN_RegisterTxIRQ(adapterInit->CAN_Number, (void*)PrivateTxIRQ, adapter);

#ifdef INC_FREERTOS_H
		adapter->TransactionMutex = xSemaphoreCreateMutex();
#endif

		//xCircleBufferInit(&adapter->RxCircleBuffer, adapterInit->RxBuffer, adapterInit->RxBufferSizeMask, sizeof(uint8_t));
		//xCircleBufferInit(&adapter->TxCircleBuffer, adapterInit->TxBuffer, adapterInit->TxBufferSizeMask, sizeof(uint8_t));

		adapter->TxHeader.StdId = 0x10 + adapterInit->CAN_Number;
		adapter->TxHeader.RTR = CAN_RTR_DATA;
		adapter->TxHeader.IDE = CAN_ID_STD;
		adapter->TxHeader.TransmitGlobalTime = DISABLE;

		CAN_FilterTypeDef filterConfig;
		filterConfig.FilterBank = adapterInit->FilterBank;
		filterConfig.FilterMode = adapterInit->FilterMode;
		filterConfig.FilterScale = adapterInit->FilterScale;
		filterConfig.FilterIdHigh = adapterInit->FilterIdHigh;
		filterConfig.FilterIdLow = adapterInit->FilterIdLow;
		filterConfig.FilterMaskIdHigh = adapterInit->FilterMaskIdHigh;
		filterConfig.FilterMaskIdLow = adapterInit->FilterMaskIdLow;
		filterConfig.FilterFIFOAssignment = adapterInit->FilterFIFOAssignment;
		filterConfig.FilterActivation = ENABLE;
		filterConfig.SlaveStartFilterBank = adapterInit->SlaveStartFilterBank;

		if(HAL_CAN_ConfigFilter(adapterInit->CAN, &filterConfig) != HAL_OK)
		{
			/* Filter configuration Error */
			Error_Handler();
		}

		HAL_CAN_ActivateNotification(adapterInit->CAN,
					CAN_IT_RX_FIFO0_MSG_PENDING
					//| CAN_IT_TX_MAILBOX_EMPTY
					| CAN_IT_ERROR
					| CAN_IT_RX_FIFO0_OVERRUN
					| CAN_IT_RX_FIFO0_FULL);

		HAL_CAN_Start(adapterInit->CAN);

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
