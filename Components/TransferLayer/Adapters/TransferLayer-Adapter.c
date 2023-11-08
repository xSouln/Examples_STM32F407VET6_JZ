//==============================================================================
//includes:

#include "TransferLayer-Adapter.h"
#include "Abstractions/xSystem/xSystem.h"
#include "Abstractions/xDevice/xService-Types.h"
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
//------------------------------------------------------------------------------
static void privateTransferResponseExchangeHandler(xTransferLayerT* layer,
		TransferLayerAdapterT* adapter,
		CAN_LocalSegmentT* segment)
{
	CAN_LocalResponseContentTransferT content = { .Value = segment->Data.Value };

	xTransferListElementT* element = (void*)layer->ProcessedTransfers.Head;
	while (element)
	{
		xTransferT* transfer = element->Value;

		if (transfer->Id == segment->TransferHeader.ServiceId
			&& transfer->Token == segment->TransferHeader.Characteristic)
		{
			transfer->Internal.OperationAttempts = transfer->TransmittingAttempts;
			transfer->Internal.OperationTimeOut = 20;

			if (transfer->Type == xTransferTypeReceive)
			{
				memcpy(transfer->Data + transfer->DataSended, content.Data, segment->DataLength - sizeof(content.Segment));
			}

			uint16_t dataSended = content.Segment * sizeof(content.Data) + sizeof(content.Data);
			transfer->DataSended = dataSended > transfer->DataLength ? transfer->DataLength : dataSended;

			transfer->Internal.RequestUpdate = true;
			TL_RxVCount++;

			break;
		}

		element = element->Next;
	}
}
//------------------------------------------------------------------------------
static void privateTransferRequestExchangeHandler(xTransferLayerT* manager,
		TransferLayerAdapterT* adapter,
		CAN_LocalSegmentT* segment)
{
	CAN_LocalRequestContentTransferT request = { .Value = segment->Data.Value };

	xTransferListElementT* element = (void*)manager->ProcessedTransfers.Head;
	while (element)
	{
		xTransferT* transfer = element->Value;

		if (transfer->Id == segment->TransferHeader.ServiceId
			&& transfer->Token == segment->TransferHeader.Characteristic)
		{
			xServiceT* holder = transfer->Holder;

			if (transfer->Type == xTransferTypeReceive)
			{
				memcpy(transfer->Data + transfer->DataSended, request.Data, segment->DataLength - sizeof(request.Segment));
			}

			if (transfer->ValidationIsEnabled)
			{
				CAN_LocalResponseContentTransferT content;
				content.Segment = request.Segment;

				CAN_LocalSegmentT packet;
				packet.TransferHeader.MessageType = CAN_LocalMessageTypeTransfer;
				packet.TransferHeader.PacketType = CAN_LocalTransferPacketTypeResponseExchange;
				packet.TransferHeader.ServiceId = holder->Id;
				packet.TransferHeader.Characteristic = segment->TransferHeader.Characteristic;
				packet.TransferHeader.IsEnabled = true;

				uint16_t size = 0;

				if (transfer->Type == xTransferTypeTransmite)
				{
					size = transfer->DataLength - transfer->DataSended;
					size = size > sizeof(content.Data) ? sizeof(content.Data) : size;

					memcpy(content.Data, transfer->Data + transfer->DataSended, size);
				}

				packet.DataLength = sizeof(content) - sizeof(content.Data) + size;
				packet.Data.Value = content.Value;

				privateTransmite(adapter->Port, &packet);
			}

			uint16_t dataSended = request.Segment * sizeof(request.Data) + sizeof(request.Data);
			transfer->DataSended = dataSended > transfer->DataLength ? transfer->DataLength : dataSended;

			transfer->Internal.TimeStamp = xSystemGetTime(NULL);
			transfer->Internal.RequestUpdate = true;

			break;
		}

		element = element->Next;
	}
}
//------------------------------------------------------------------------------
static void privateApproveTransferHandler(xTransferLayerT* layer,
		TransferLayerAdapterT* adapter,
		CAN_LocalSegmentT* segment)
{
	CAN_LocalResponseContentOpenTransferT response = { .Value = segment->Data.Value };

	xTransferListElementT* element = (void*)layer->ProcessedTransfers.Head;

	while (element)
	{
		xTransferT* transfer = element->Value;
		xServiceT* holder = transfer->Holder;

		if (transfer->Id == segment->TransferHeader.ServiceId
			&& response.ServiceId == holder->Id
			&& response.Result == xResultAccept)
		{
			transfer->Internal.TimeStamp = xSystemGetTime(NULL);

			transfer->Internal.OperationAttempts = 3;
			transfer->Internal.OperationTimeOut = 20;
			transfer->Internal.RequestUpdate = true;
			transfer->Token = response.Token;

			if (transfer->Type == xTransferTypeReceive)
			{
				transfer->State = transfer->ValidationIsEnabled ? xTransferStateTransmitting : xTransferStateReceiving;
			}
			else if (transfer->Type == xTransferTypeTransmite || transfer->Type == xTransferTypeTransmiteReceive)
			{
				transfer->State = xTransferStateTransmitting;
			}

			TL_RxCount = 0;
			TL_TxCount = 0;
			TL_RxVCount = 0;

			break;
		}

		element = element->Next;
	}

}
//------------------------------------------------------------------------------
static void privateTranferHandler(xTransferLayerT* layer,
		TransferLayerAdapterT* adapter)
{
	xTransferListElementT* element = xListStartEnumeration((void*)&layer->ProcessedTransfers);

	while (element)
	{
		xTransferT* transfer = element->Value;
		void* deletedElement = NULL;

		uint32_t time = xSystemGetTime(NULL);

		if (transfer->Internal.RequestUpdate)
		{
			transfer->Internal.RequestUpdate = false;
			goto update;
		}

		if (time - transfer->Internal.TimeStamp < transfer->Internal.OperationTimeOut)
		{
			goto end;
		}

		if (time - transfer->Internal.TimeStamp >= transfer->TimeOut)
		{
			if (transfer->TransmittingAttempts)
			{
				transfer->TransmittingAttempts--;
			}
			else
			{
				deletedElement = transfer;
				transfer->Result = xTransferResultTimeOut;
				transfer->IsRunning = false;

				if (transfer->EventListener)
				{
					transfer->EventListener(layer, transfer, xTransferEventError, NULL);
				}
			}
		}

		update:;
		switch ((uint8_t)transfer->State)
		{
			case xTransferStatePending:
			{
				break;
			}

			case xTransferStateTransmitting:
			{
				if (transfer->DataSended == transfer->DataLength)
				{
					transfer->Internal.OperationTimeOut = 0;
					transfer->State = xTransferStateCompleted;
				}
				else if (transfer->MasterModeIsEnabled || !transfer->ValidationIsEnabled)
				{
					uint32_t freeBufferSize = xPortGetTxBufferFreeSize(adapter->Port);

					if (!freeBufferSize)
					{
						break;
					}

					xServiceT* holder = transfer->Holder;

					CAN_LocalSegmentT packet = { 0 };
					packet.TransferHeader.IsEnabled = true;
					packet.TransferHeader.MessageType = CAN_LocalMessageTypeTransfer;
					packet.TransferHeader.PacketType = CAN_LocalTransferPacketTypeRequestExchange;
					packet.TransferHeader.ServiceId = holder->Id;
					packet.TransferHeader.Characteristic = transfer->Token;

					if (transfer->ValidationIsEnabled)
					{
						packet.TransferHeader.PacketType = CAN_LocalTransferPacketTypeRequestExchange;

						CAN_LocalRequestContentTransferT content;
						content.Segment = transfer->DataSended / sizeof(content.Data);

						int size = 0;

						if (transfer->Type != xTransferTypeReceive)
						{
							size = transfer->DataLength - transfer->DataSended;
							size = size > sizeof(content.Data) ? sizeof(content.Data) : size;

							memcpy(content.Data, transfer->Data + transfer->DataSended, size);
						}

						packet.DataLength = sizeof(content) - sizeof(content.Data) + size;
						packet.Data.Value = content.Value;

						//xPortExtendedTransmition(adapter->Port, &packet);
						privateTransmite(adapter->Port, &packet);

						transfer->Internal.TimeStamp = time;
						transfer->Internal.OperationTimeOut = 20;

						break;
					}

					while (freeBufferSize > 10 && transfer->DataSended != transfer->DataLength)
					{
						CAN_LocalRequestContentTransferT content;
						content.Segment = transfer->DataSended / sizeof(content.Data);

						uint16_t size = 0;

						if (transfer->Type != xTransferTypeReceive)
						{
							size = transfer->DataLength - transfer->DataSended;
							size = size > sizeof(content.Data) ? sizeof(content.Data) : size;

							memcpy(content.Data, transfer->Data + transfer->DataSended, size);
						}

						packet.DataLength = sizeof(content) - sizeof(content.Data) + size;
						packet.Data.Value = content.Value;

						//xPortExtendedTransmition(adapter->Port, &packet);
						privateTransmite(adapter->Port, &packet);

						transfer->DataSended += size;

						freeBufferSize--;

						transfer->Internal.TimeStamp = time;
						transfer->Internal.OperationTimeOut = 1;

						break;
					}
				}

				break;
			}

			case xTransferStateReceiving:
			{
				if (transfer->DataSended != transfer->DataLength)
				{
					break;
				}

				transfer->State = xTransferStateCompleted;
			}

			case xTransferStateCompleted:
			{
				deletedElement = transfer;

				transfer->Result = xTransferResultNoError;
				transfer->State = xTransferStateIdle;
				transfer->Completed = true;
				transfer->IsRunning = false;
				transfer->TransmissionTime = xSystemGetTime(NULL) - transfer->Internal.StartTransmissionTimeStamp;

				transfer->EventListener(layer, transfer, xTransferEventComplite, NULL);
				break;
			}

			default:
				break;
		}

		end:;
		element = element->Next;

		if (deletedElement)
		{
			xListRemove((void*)&layer->ProcessedTransfers, deletedElement);
		}
	}

	xListStopEnumeration((void*)&layer->ProcessedTransfers);
}
//------------------------------------------------------------------------------
static void privateHandler(xTransferLayerT* manager)
{
	TransferLayerAdapterT* adapter = (TransferLayerAdapterT*)manager->Adapter.Content;

	while (adapter->Content.RxPacketHandlerIndex != adapter->Content.PortRxCircleBuffer->TotalIndex)
	{
		CAN_LocalSegmentT* segment = xCircleBufferGetElement(adapter->Content.PortRxCircleBuffer, adapter->Content.RxPacketHandlerIndex);

		TL_RxCount++;

		if (segment->ExtensionIsEnabled)
		{
			uint8_t messageType = segment->MessageType;

			switch (messageType)
			{
				case CAN_LocalMessageTypeTransfer:
				{
					switch ((uint8_t)segment->TransferHeader.PacketType)
					{
						case CAN_LocalTransferPacketTypeApproveTransfer:
							privateApproveTransferHandler(manager, adapter, segment);
							break;

						case CAN_LocalTransferPacketTypeResponseExchange:
							privateTransferResponseExchangeHandler(manager, adapter, segment);
							break;

						case CAN_LocalTransferPacketTypeRequestExchange:
							privateTransferRequestExchangeHandler(manager, adapter, segment);
							break;
					}

					break;
				}
			}
		}

		adapter->Content.RxPacketHandlerIndex++;
		adapter->Content.RxPacketHandlerIndex &= adapter->Content.PortRxCircleBuffer->SizeMask;
	}

	privateTranferHandler(manager, adapter);
}
//------------------------------------------------------------------------------
static xResult privateRequestAdd(xTransferLayerT* manager, TransferLayerAdapterT* adapter, xTransferT* transfer)
{
	xSemaphoreTake(adapter->Content.CoreMutex, portMAX_DELAY);

	if (transfer->State == xTransferStateIdle || transfer->State == xTransferStatePreparing)
	{
		transfer->IsRunning = true;
		transfer->Completed = false;
		transfer->DataSended = 0;
		transfer->TransmissionTime = 0;
		transfer->Internal.StartTransmissionTimeStamp = xSystemGetTime(NULL);

		if (transfer->MasterModeIsEnabled)
		{
			transfer->Internal.OperationTimeOut = 100;
			transfer->Internal.OperationAttempts = 1;

			xServiceT* holder = transfer->Holder;
			CAN_LocalTransferT* extansion = (void*)transfer;

			CAN_LocalRequestContentOpenTransferT content;
			content.ServiceId = transfer->Id;
			content.Action = extansion->Action;
			content.Type = transfer->Type;
			content.ValidationIsEnabled = transfer->ValidationIsEnabled;

			CAN_LocalSegmentT packet;
			packet.TransferHeader.MessageType = CAN_LocalMessageTypeTransfer;
			packet.TransferHeader.PacketType = CAN_LocalTransferPacketTypeOpenTransfer;
			packet.TransferHeader.Characteristic = holder->Info.Type;
			packet.TransferHeader.ServiceId = holder->Id;
			packet.TransferHeader.IsEnabled = true;

			packet.Data.Value = content.Value;
			packet.DataLength = sizeof(CAN_LocalRequestContentOpenTransferT);

			privateTransmite(adapter->Port, &packet);

			transfer->State = xTransferStateOpening;
		}
		else if (transfer->Type == xTransferTypeReceive)
		{
			transfer->State = xTransferStateReceiving;
		}
		else if (transfer->Type == xTransferTypeTransmite)
		{
			transfer->State = xTransferStateTransmitting;
		}

		transfer->Internal.TimeStamp = xSystemGetTime(NULL);
		xListAdd((void*)&manager->ProcessedTransfers, transfer);
	}

	xSemaphoreGive(adapter->Content.CoreMutex);

	return xResultBusy;
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

		case xTransferLayerAdapterRequestAdd:
			privateRequestAdd(manager, adapter, arg);
			break;


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

	adapter->Port = init->Port;

	adapter->Content.PortRxCircleBuffer = xPortGetRxCircleBuffer(adapter->Port);

	return xResultError;
}
//==============================================================================
