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
	CAN_LocalPacketTransferResponseT content = { .Value = segment->Data.DoubleWord };

	xTransferListElementT* element = (void*)layer->ProcessedTransfers.Head;
	while (element)
	{
		xTransferT* transfer = element->Value;

		if (transfer->Id == segment->TransferHeader.ServiceId)
		{
			transfer->Internal.OperationAttempts = transfer->TransmittingAttempts;
			transfer->Internal.OperationTimeOut = 20;

			uint16_t dataSended = segment->TransferHeader.Characteristic * sizeof(content.Data) + sizeof(content.Data);
			transfer->DataSended = dataSended > transfer->DataLength ? transfer->DataLength : dataSended;

			transfer->Internal.RequestUpdate = true;
			TL_RxVCount++;
		}

		element = element->Next;
	}
}
//------------------------------------------------------------------------------
static void privateTransferRequestExchangeHandler(xTransferLayerT* manager,
		TransferLayerAdapterT* adapter,
		CAN_LocalSegmentT* segment)
{
	CAN_LocalPacketTransferRequestT request = { .Value = segment->Data.DoubleWord };

	xTransferListElementT* element = (void*)manager->ProcessedTransfers.Head;
	while (element)
	{
		xTransferT* transfer = element->Value;

		if (transfer->Id == segment->TransferHeader.ServiceId)
		{
			xServiceT* holder = transfer->Holder;

			CAN_LocalPacketTransferResponseT content;

			int size = transfer->DataLength - transfer->DataSended;
			size = size > sizeof(content.Data) ? sizeof(content.Data) : size;

			CAN_LocalSegmentT packet;
			packet.TransferHeader.MessageType = CAN_LocalMessageTypeTransfer;
			packet.TransferHeader.PacketType = CAN_LocalTransferPacketTypeResponseExchange;
			packet.TransferHeader.ServiceId = holder->Id;
			packet.TransferHeader.Characteristic = segment->TransferHeader.Characteristic;
			packet.TransferHeader.IsEnabled = true;
			packet.DataLength = 0;

			memcpy(transfer->Data + transfer->DataSended, request.Data, segment->DataLength);

			uint16_t dataSended = segment->TransferHeader.Characteristic * sizeof(content.Data) + sizeof(content.Data);
			transfer->DataSended = dataSended > transfer->DataLength ? transfer->DataLength : dataSended;

			privateTransmite(adapter->Port, &packet);

			transfer->Internal.TimeStamp = xSystemGetTime(NULL);
			transfer->Internal.RequestUpdate = true;
		}

		element = element->Next;
	}
}
//------------------------------------------------------------------------------
static void privateApproveTransferHandler(xTransferLayerT* layer,
		TransferLayerAdapterT* adapter,
		CAN_LocalSegmentT* segment)
{
	volatile CAN_LocalPacketOpenTransferResponseT response = { .Value = segment->Data.Value };

	xTransferListElementT* element = (void*)layer->ProcessedTransfers.Head;

	while (element)
	{
		xTransferT* transfer = element->Value;

		if (transfer->Id == segment->TransferHeader.ServiceId
			&& response.Result == xResultAccept
			&& transfer->Token == response.Token)
		{
			transfer->Internal.TimeStamp = xSystemGetTime(NULL);
			transfer->State = transfer->Type == xTransferTypeReceive ? xTransferStateReceiving : xTransferStateTransmitting;

			transfer->Internal.OperationAttempts = 3;
			transfer->Internal.OperationTimeOut = 20;
			transfer->Internal.RequestUpdate = true;

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

				if (transfer->EventAccomplish)
				{
					transfer->EventAccomplish(layer, transfer);
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
					transfer->State = xTransferStateCompleted;
				}
				else
				{
					xServiceT* holder = transfer->Holder;

					CAN_LocalSegmentT packet = { 0 };
					packet.TransferHeader.IsEnabled = true;
					packet.TransferHeader.MessageType = CAN_LocalMessageTypeTransfer;
					packet.TransferHeader.PacketType = CAN_LocalTransferPacketTypeRequestExchange;
					packet.TransferHeader.ServiceId = holder->Id;

					CAN_LocalPacketTransferRequestT content;
					packet.TransferHeader.Characteristic = transfer->DataSended / sizeof(content.Data);

					int size = transfer->DataLength - transfer->DataSended;
					size = size > sizeof(content.Data) ? sizeof(content.Data) : size;

					memcpy(content.Data, transfer->Data + transfer->DataSended, size);

					packet.DataLength = sizeof(content) - sizeof(content.Data) + size;
					packet.Data.Value = content.Value;

					//xPortExtendedTransmition(adapter->Port, &packet);
					privateTransmite(adapter->Port, &packet);

					transfer->Internal.TimeStamp = time;
					transfer->Internal.OperationTimeOut = 20;

					break;
				}
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
				transfer->EventAccomplish(layer, transfer);
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

		if (transfer->Type == xTransferTypeTransmite)
		{
			transfer->Internal.OperationTimeOut = 100;
			transfer->Internal.OperationAttempts = 1;

			xServiceT* holder = transfer->Holder;

			CAN_LocalPacketOpenTransferRequestT content;
			content.ServiceId = transfer->Id;
			content.Id = 0;
			content.Token = transfer->Token;
			content.Type = transfer->Type;
			content.ValidationIsEnabled = transfer->ValidationIsEnabled;

			CAN_LocalSegmentT packet;
			packet.ExtensionHeader.MessageType = CAN_LocalMessageTypeTransfer;
			packet.ExtensionHeader.PacketType = CAN_LocalTransferPacketTypeOpenTransfer;
			packet.ExtensionHeader.ServiceType = holder->Info.Type;
			packet.ExtensionHeader.ServiceId = holder->Id;
			packet.ExtensionIsEnabled = true;

			packet.Data.DoubleWord = content.Value;
			packet.DataLength = sizeof(CAN_LocalPacketOpenTransferRequestT);

			privateTransmite(adapter->Port, &packet);

			transfer->State = xTransferStateOpening;
			transfer->Internal.TimeStamp = xSystemGetTime(NULL);
		}
		else if (transfer->Type == xTransferTypeReceive && transfer->ValidationIsEnabled)
		{
			transfer->State = xTransferStateReceiving;
			transfer->Internal.TimeStamp = xSystemGetTime(NULL);
		}

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

		/*case xTransferLayerAdapterRequestExecuteCommand:
			xSemaphoreTake(adapter->Content.CoreMutex, portMAX_DELAY);

			adapter->Content.Command = arg;
			adapter->Content.CommandArgs = (&arg + 1);

			adapter->Content.CommandResult = adapter->Content.Command(manager, adapter->Content.CommandArgs);
			adapter->Content.Command = NULL;

			//xSemaphoreTake(adapter->Content.CommandAccomplishSemaphore, portMAX_DELAY);
			xSemaphoreGive(adapter->Content.CoreMutex);

			return adapter->Content.CommandResult;*/

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
