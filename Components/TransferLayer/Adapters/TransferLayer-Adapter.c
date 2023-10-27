//==============================================================================
//includes:

#include "TransferLayer-Adapter.h"
#include "Abstractions/xSystem/xSystem.h"
//==============================================================================
//defines:


//==============================================================================
//types:


//==============================================================================
//variables:

static uint32_t TL_TxCount;
static uint32_t TL_RxCount;
static uint32_t TL_RxVCount;
//==============================================================================
//functions:

static void privateTransmite(xPortT* port, void* packet)
{
	TL_TxCount++;

	xPortExtendedTransmition(port, packet);
}

static void privateRxTransferHandler(xTransferLayerT* manager,
		TransferLayerAdapterT* adapter,
		CAN_LocalSegmentT* segment)
{
	CAN_LocalPacketTransactionResponseT content = { .Value = segment->Data.DoubleWord };

	xTransferListElementT* element = (void*)manager->ProcessedTransfers.Head;
	while (element)
	{
		xTransferT* transaction = element->Value;

		if (transaction->Token == content.Token)
		{
			transaction->TimeStamp = xSystemGetTime(NULL);
			transaction->Token = content.Token;

			transaction->TransmittingAttempts = 3;
			transaction->TimeOut = 20;

			uint16_t dataSended = content.Segment * sizeof(content.Data) + sizeof(content.Data);
			transaction->DataSended = dataSended > transaction->DataLength ? transaction->DataLength : dataSended;

			transaction->RequestUpdate = true;
			TL_RxVCount++;

			(void)segment;

			//memcpy(transaction->Data + response.Segment * sizeof(response.Data), )

			//transaction->State = xTransferStateProcessing;
		}

		element = element->Next;
	}
}
//------------------------------------------------------------------------------
static void privateTxTransferHandler(xTransferLayerT* manager,
		TransferLayerAdapterT* adapter,
		CAN_LocalSegmentT* segment)
{
	CAN_LocalPacketTransactionRequestT request = { .Value = segment->Data.DoubleWord };

	xTransferListElementT* element = (void*)manager->ProcessedTransfers.Head;
	while (element)
	{
		xTransferT* transaction = element->Value;

		if (segment->ExtensionHeader.Address == 2000 && transaction->Token == request.Token)
		{
			CAN_LocalPacketTransactionResponseT content;
			content.Token = transaction->Token;
			content.Segment = request.Segment;

			int size = transaction->DataLength - transaction->DataSended;
			size = size > sizeof(content.Data) ? sizeof(content.Data) : size;

			CAN_LocalSegmentT packet;
			packet.ExtensionHeader.MessageType = CAN_LocalMessageTypeResponse;
			packet.ExtensionHeader.PacketType = CAN_LocalPacketTypeTransaction;
			packet.ExtensionHeader.Address = 2001;
			packet.ExtensionIsEnabled = true;
			packet.Data.DoubleWord = content.Value;
			packet.DataLength = sizeof(request.Token) + sizeof(request.Segment);

			transaction->TimeStamp = xSystemGetTime(NULL);
			transaction->RequestUpdate = true;

			uint16_t dataSended = request.Segment * sizeof(content.Data) + sizeof(content.Data);
			uint16_t dataSize = segment->DataLength - sizeof(request.Token) - sizeof(request.Segment);

			memcpy(transaction->Data + transaction->DataSended, request.Data, dataSize);

			transaction->DataSended = dataSended > transaction->DataLength ? transaction->DataLength : dataSended;
			//xPortExtendedTransmition(adapter->Port, &packet);
			privateTransmite(adapter->Port, &packet);
		}

		element = element->Next;
	}
}
//------------------------------------------------------------------------------
static void privateApproveTransactionHandler(xTransferLayerT* manager,
		TransferLayerAdapterT* adapter,
		CAN_LocalSegmentT* segment)
{
	CAN_LocalPacketOpenTransactionResponseT response = { .Value = segment->Data.DoubleWord };

	xTransferListElementT* element = (void*)manager->ProcessedTransfers.Head;

	while (element)
	{
		xTransferT* transaction = element->Value;

		if (transaction->Id == response.DeviceId && transaction->Token == response.Token)
		{
			transaction->TimeStamp = xSystemGetTime(NULL);
			transaction->Token = response.Token;
			transaction->State = transaction->Type == xTransferTypeReceive ? xTransferStateReceiving : xTransferStateTransmitting;

			transaction->TransmittingAttempts = 3;
			transaction->TimeOut = 20;

			transaction->RequestUpdate = true;

			TL_RxCount = 0;
			TL_TxCount = 0;
			TL_RxVCount = 0;
			break;
		}

		element = element->Next;
	}

}
//------------------------------------------------------------------------------
static void privateTransactionHandler(xTransferLayerT* manager,
		TransferLayerAdapterT* adapter)
{
	xTransferListElementT* element = xListStartEnumeration((void*)&manager->ProcessedTransfers);

	while (element)
	{
		xTransferT* transaction = element->Value;
		void* deletedElement = NULL;

		uint32_t time = xSystemGetTime(NULL);

		if (transaction->RequestUpdate)
		{
			transaction->RequestUpdate = false;
			transaction->WaitOperation = false;
			goto update;
		}

		if (transaction->WaitOperation)
		{
			if (time - transaction->TimeStamp < transaction->TimeOut)
			{
				goto end;
			}

			transaction->WaitOperation = false;
		}

		if ((uint8_t)transaction->State != xTransferStatePending
			&& time - transaction->TimeStamp >= transaction->TimeOut)
		{
			if (transaction->TransmittingAttempts)
			{
				transaction->TransmittingAttempts--;
			}
			else
			{
				deletedElement = transaction;
				transaction->Result = xTransferResultTimeOut;

				if (transaction->EventAccomplish)
				{
					transaction->EventAccomplish(transaction);
				}
			}
		}

		update:;
		switch ((uint8_t)transaction->State)
		{
			case xTransferStatePending:
			{
				CAN_LocalPacketOpenTransactionRequestT content;
				content.DeviceId = 2000;
				content.ServiceId = 22;
				content.Action = 2;
				content.Token = 69;

				CAN_LocalSegmentT packet;
				packet.ExtensionHeader.MessageType = CAN_LocalMessageTypeRequest;
				packet.ExtensionHeader.PacketType = CAN_LocalPacketTypeOpenTransaction;
				packet.ExtensionHeader.DeviceType = 11;
				packet.ExtensionHeader.Address = content.DeviceId;
				packet.ExtensionIsEnabled = true;

				packet.Data.DoubleWord = content.Value;
				packet.DataLength = sizeof(CAN_LocalPacketOpenTransactionRequestT);

				//xPortExtendedTransmition(adapter->Port, &packet);
				privateTransmite(adapter->Port, &packet);

				transaction->Id = content.DeviceId;
				transaction->Token = content.Token;
				transaction->TimeStamp = time;
				transaction->State = xTransferStateOpening;
				transaction->WaitOperation = true;

				break;
			}

			case xTransferStateTransmitting:
			{
				if (transaction->DataSended == transaction->DataLength)
				{
					transaction->State = xTransferStateCompleted;
				}
				else
				{
					CAN_LocalSegmentT packet = { 0 };
					packet.ExtensionHeader.MessageType = CAN_LocalMessageTypeRequest;
					packet.ExtensionHeader.PacketType = CAN_LocalPacketTypeTransaction;
					packet.ExtensionHeader.DeviceType = 11;
					packet.ExtensionHeader.Address = 2000;
					packet.ExtensionIsEnabled = true;

					CAN_LocalPacketTransactionRequestT content;
					content.Token = transaction->Token;
					content.Segment = transaction->DataSended / sizeof(content.Data);

					int size = transaction->DataLength - transaction->DataSended;
					size = size > sizeof(content.Data) ? sizeof(content.Data) : size;

					memcpy(content.Data, transaction->Data + transaction->DataSended, size);

					packet.DataLength = sizeof(content) - sizeof(content.Data) + size;
					packet.Data.DoubleWord = content.Value;

					//xPortExtendedTransmition(adapter->Port, &packet);
					privateTransmite(adapter->Port, &packet);

					transaction->TimeStamp = time;
					transaction->WaitOperation = true;

					break;
				}
			}

			case xTransferStateReceiving:
			{
				if (transaction->DataSended != transaction->DataLength)
				{
					break;
				}

				transaction->State = xTransferStateCompleted;
			}

			case xTransferStateCompleted:
			{
				deletedElement = transaction;
				transaction->Result = xTransferResultNoError;
				transaction->Completed = true;
				transaction->State = xTransferStateIdle;
				transaction->EventAccomplish(transaction);
				break;
			}
		}

		end:;
		element = element->Next;

		if (deletedElement)
		{
			xListRemove((void*)&manager->ProcessedTransfers, deletedElement);
		}
	}

	xListStopEnumeration((void*)&manager->ProcessedTransfers);
}
//------------------------------------------------------------------------------
static void privateMessageTypeResponseHandler(xTransferLayerT* manager,
		TransferLayerAdapterT* adapter,
		CAN_LocalSegmentT* segment)
{
	if (segment->ExtensionIsEnabled)
	{
		switch (segment->ExtensionHeader.PacketType)
		{
			case CAN_LocalPacketTypeTransaction:
				privateRxTransferHandler(manager, adapter, segment);
				break;

			case CAN_LocalPacketTypeApproveTransaction:
				privateApproveTransactionHandler(manager, adapter, segment);
				break;

			default: break;
		}
	}
}
//------------------------------------------------------------------------------
static void privateMessageTypeRequestHandler(xTransferLayerT* manager,
		TransferLayerAdapterT* adapter,
		CAN_LocalSegmentT* segment)
{
	if (segment->ExtensionIsEnabled)
	{
		switch (segment->ExtensionHeader.PacketType)
		{
			case CAN_LocalPacketTypeTransaction:
				privateTxTransferHandler(manager, adapter, segment);
				break;

			default: break;
		}
	}
}
//------------------------------------------------------------------------------
static void privateHandler(xTransferLayerT* manager)
{
	TransferLayerAdapterT* adapter = (TransferLayerAdapterT*)manager->Adapter.Content;

	/*if (adapter->Content.Command)
	{
		adapter->Content.CommandResult = adapter->Content.Command(manager, adapter->Content.CommandArgs);
		adapter->Content.Command = NULL;

		xSemaphoreGive(adapter->Content.CommandAccomplishSemaphore);
	}*/

	while (adapter->Content.RxPacketHandlerIndex != adapter->Content.PortRxCircleBuffer->TotalIndex)
	{
		CAN_LocalSegmentT* segment = xCircleBufferGetElement(adapter->Content.PortRxCircleBuffer, adapter->Content.RxPacketHandlerIndex);

		TL_RxCount++;

		if (segment->ExtensionIsEnabled)
		{
			uint8_t messageType = segment->ExtensionHeader.MessageType;
			uint8_t packetType = segment->ExtensionHeader.PacketType;

			switch (messageType)
			{
				case CAN_LocalMessageTypeRequest:
					privateMessageTypeRequestHandler(manager, adapter, segment);
					break;

				case CAN_LocalMessageTypeResponse:
					switch (packetType)
					{
						case CAN_LocalPacketTypeTransaction:
							privateRxTransferHandler(manager, adapter, segment);
							break;

						case CAN_LocalPacketTypeApproveTransaction:
							privateApproveTransactionHandler(manager, adapter, segment);
							break;

						default: break;
					}
					break;
			}
		}

		adapter->Content.RxPacketHandlerIndex++;
		adapter->Content.RxPacketHandlerIndex &= adapter->Content.PortRxCircleBuffer->SizeMask;
	}

	privateTransactionHandler(manager, adapter);
}
//------------------------------------------------------------------------------
static xResult privateRequestListener(xTransferLayerT* manager, xTransferLayerAdapterRequestSelector selector, void* arg, ...)
{
	TransferLayerAdapterT* adapter = (TransferLayerAdapterT*)manager->Adapter.Content;

	switch ((uint32_t)selector)
	{
		case xTransferLayerAdapterRequestLock:
			xSemaphoreTake(adapter->Content.CoreMutex, portMAX_DELAY);
			break;

		case xTransferLayerAdapterRequestUnLock:
			xSemaphoreGive(adapter->Content.CoreMutex);
			break;

		case xTransferLayerAdapterRequestExecuteCommand:
			xSemaphoreTake(adapter->Content.CoreMutex, portMAX_DELAY);

			adapter->Content.Command = arg;
			adapter->Content.CommandArgs = (&arg + 1);

			adapter->Content.CommandResult = adapter->Content.Command(manager, adapter->Content.CommandArgs);
			adapter->Content.Command = NULL;

			//xSemaphoreTake(adapter->Content.CommandAccomplishSemaphore, portMAX_DELAY);
			xSemaphoreGive(adapter->Content.CoreMutex);

			return adapter->Content.CommandResult;

		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//==============================================================================
//initializations:

static xTransferLayerAdapterInterfaceT privateAdapterInterface =
{
	.Handler = (xTransferLayerAdapterHandlerT)privateHandler,
	.RequestListener = (xTransferLayerAdapterRequestListenerT)privateRequestListener,
};
//------------------------------------------------------------------------------
xResult TransferLayerAdapterInit(xTransferLayerT* manager,
		TransferLayerAdapterT* adapter,
		TransferLayerAdapterInitT* init)
{
	manager->Adapter.Description = nameof(TransferLayerAdapterT);
	manager->Adapter.Content = adapter;
	manager->Adapter.Interface = &privateAdapterInterface;

	adapter->Content.CoreMutex = xSemaphoreCreateMutex();
	adapter->Content.CommandAccomplishSemaphore = xSemaphoreCreateBinary();
	adapter->Port = init->Port;

	adapter->Content.PortRxCircleBuffer = xPortGetRxCircleBuffer(adapter->Port);

	return xResultError;
}
//==============================================================================
