//==============================================================================
//includes:

#include "CAN_LocalPort-Adapter.h"
//==============================================================================
//defines:


//==============================================================================
//types:


//==============================================================================
//variables:

static uint32_t CAN_LocalRequestsCount;
static uint32_t CAN_LocalResponsesCount;
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

	xSemaphoreTake(adapter->TxSemaphore, 1);

	if (!adapter->AwaitTxValidation)
	{
		if (adapter->TxValidationComplite)
		{
			adapter->TxValidationComplite = false;

			if (!adapter->TxError)
			{
				xCircleBufferOffsetHandlerIndex(&adapter->TxCircleBuffer, 1);
			}
		}

		if (adapter->TxCircleBuffer.HandlerIndex != adapter->TxCircleBuffer.TotalIndex)
		{
			CAN_LocalSegmentT* segment = ((CAN_LocalSegmentT*)adapter->TxCircleBuffer.Memory) + adapter->TxCircleBuffer.HandlerIndex;

			uint32_t txMailbox;

			if (!segment->ExtensionIsEnabled)
			{
				adapter->TxHeader.StdId = segment->Identifier;
				adapter->TxHeader.IDE = CAN_ID_STD;
			}
			else
			{
				adapter->TxHeader.ExtId = segment->Identifier;
				adapter->TxHeader.ExtId <<= 18;
				adapter->TxHeader.ExtId |= segment->Extension;
				adapter->TxHeader.IDE = CAN_ID_EXT;
			}

			adapter->TxHeader.DLC = segment->DataLength;

			adapter->TxError = 0;
			adapter->AwaitTxValidation = true;
			adapter->TxValidationComplite = false;

			if (HAL_CAN_AddTxMessage(adapter->CAN, &adapter->TxHeader, segment->Data.Bytes, &txMailbox) != HAL_OK)
			{
				adapter->AwaitTxValidation = false;
				return;
			}

			CAN_LocalRequestsCount++;
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
		CAN_LocalSegmentT segment;
		segment.Identifier = reg->RxMailBox[0].Identifier.StandardIdentifier;
		segment.Extension = reg->RxMailBox[0].Identifier.ExtendedIdentifie;
		segment.ExtensionIsEnabled = reg->RxMailBox[0].Identifier.IdentifierExtension;
		segment.DataLength = reg->RxMailBox[0].DataControlAndTimeStamp.DataLengthCode;

		segment.Data.Words[0] = reg->RxMailBox[0].DataLow.Value;
		segment.Data.Words[1] = reg->RxMailBox[0].DataHigh.Value;

		xCircleBufferAddObject(&adapter->RxCircleBuffer, &segment, 1, 0, 0);

		reg->RxFIFO0.ReleaseOutputMailbox = true;

		CAN_LocalResponsesCount++;
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

	BaseType_t yield = pdFALSE;
	xSemaphoreGiveFromISR(adapter->TxSemaphore, &yield);
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


			case xPortAdapterRequestGetRxCircleBuffer:
			{
				xCircleBufferT** out = arg;
				*out = &adapter->RxCircleBuffer;

				break;
			}

			case xPortAdapterRequesExtendedTransmission:
			{
				xCircleBufferAddObject(&adapter->TxCircleBuffer, arg, 1, 0, 0);
				xSemaphoreGive(adapter->TxSemaphore);
				break;
			}

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
		adapter->TxSemaphore = xSemaphoreCreateBinary();
#endif

		xCircleBufferInit(&adapter->RxCircleBuffer, adapterInit->RxBuffer, adapterInit->RxBufferSizeMask, sizeof(CAN_LocalSegmentT));
		xCircleBufferInit(&adapter->TxCircleBuffer, adapterInit->TxBuffer, adapterInit->TxBufferSizeMask, sizeof(CAN_LocalSegmentT));

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
