//==============================================================================
//includes:

#include "TransferLayer-Component.h"
#include "CAN_Local/CAN_Local-Component.h"

#include "CAN_Local/Control/CAN_Local-Types.h"
#include "Abstractions/xTransferLayer/xTransferLayer.h"
#include "Abstractions/xSystem/xSystem.h"
#include "Adapters/TransferLayer-Adapter.h"

#include "Components.h"
//==============================================================================
//defines:

#define CAN_LOCAL_TRANSFER_BUFFER_SIZE 20
#define CAN_EXTERNAL_TRANSFER_BUFFER_SIZE 20
//==============================================================================
//import:


//==============================================================================
//variables:

static CAN_LocalTransferT CAN_LocalTransfersBuffer[CAN_LOCAL_TRANSFER_BUFFER_SIZE];
static CAN_LocalTransferT CAN_ExternalTransfersBuffer[CAN_EXTERNAL_TRANSFER_BUFFER_SIZE];

xTransferLayerT LocalTransferLayer;
xTransferLayerT ExternalTransferLayer;

static uint32_t privateTimeStamp;
static uint32_t privateTimeStamp1;

static uint32_t privateBeginningTransferTimeStamp;
static uint32_t privateBeginningTransferTimeDifference;

static CAN_LocalTransferT privateTransfer;

const uint8_t transferTxData[] = "11223344556677889900112233445566778111111111111111111111111111111111111642724574864978764363474626326358888888888888843566313453452363568679567983563253agdgasdbjfDFSDFASGHA89900"
		"11223344556677889900112233445566778111111111111111111111111111111111111642724574864978764363474626326358888888888888843566313453452363568679567983563253agdgasdbjfDFSDFASGHA89900"
		"11223344556677889900112233445566778111111111111111111111111111111111111642724574864978764363474626326358888888888888843566313453452363568679567983563253agdgasdbjfDFSDFASGHA89900";
uint8_t transferRxData[sizeof_str(transferTxData)];
//==============================================================================
//functions:

static void transferComplite(xTransferLayerT* layer, xTransferT* transfer)
{
	transfer->State = xTransferStateIdle;
	privateBeginningTransferTimeDifference = transfer->Internal.TimeStamp - privateBeginningTransferTimeStamp;
}
//------------------------------------------------------------------------------
static void privateEventAccomplish(xTransferLayerT* layer, xTransferT* transfer)
{
	transfer->State = xTransferStateIdle;

	if (memcmp(transferTxData, transferRxData, sizeof(transferRxData)) != 0)
	{

	}
}
//------------------------------------------------------------------------------
void TransferLayerComponentHandler()
{
	xTransferLayerHandlerDirect(LocalTransferLayer);
	xTransferLayerHandlerDirect(ExternalTransferLayer);

	//return;

	uint32_t time = xSystemGetTime(NULL);
	if (time - privateTimeStamp > 2000)
	{
		privateTimeStamp = time;

		if (privateTransfer.Base.State == xTransferStateIdle)
		{
			privateTransfer.Base.Holder = LocalDevice.Services.Head->Value;
			privateTransfer.Base.Id = TemperatureService3.Base.Id;
			privateTransfer.Base.Type = xTransferTypeTransmite;
			privateTransfer.Base.ValidationIsEnabled = true;

			privateTransfer.Base.Data = (uint8_t*)transferTxData;
			privateTransfer.Base.DataLength = sizeof_str(transferTxData);
			privateTransfer.Base.Token = 3;
			privateTransfer.Base.TimeOut = 1000;
			privateTransfer.Base.EventAccomplish = transferComplite;
			privateTransfer.Base.TransmittingAttempts = 1;

			xTransferLayerAdd(&LocalTransferLayer, (void*)&privateTransfer);
			privateBeginningTransferTimeStamp = xSystemGetTime(NULL);
		}

		xTransferT* transfer = xTransferLayerNewTransfer(&ExternalTransferLayer);

		if (transfer)
		{
			transfer->Holder = &TemperatureService3;
			transfer->Id = LocalDevice.Id;
			transfer->Type = xTransferTypeReceive;
			transfer->ValidationIsEnabled = true;

			transfer->Data = transferRxData;
			transfer->DataLength = sizeof(transferRxData);
			transfer->Token = 3;
			transfer->TimeOut = 1000;
			transfer->TransmittingAttempts = 1;

			transfer->EventAccomplish = privateEventAccomplish;

			memset(transferRxData, 0, sizeof(transferRxData));

			xTransferLayerAdd(&ExternalTransferLayer, transfer);
		}
	}

	if (time - privateTimeStamp1 > 10)
	{
		privateTimeStamp1 = time;

		CAN_LocalSegmentT packet;
		packet.ExtensionHeader.MessageType = CAN_LocalMessageTypeNotification;
		packet.ExtensionHeader.ServiceType = 11;
		packet.ExtensionHeader.ServiceId = 333;
		packet.ExtensionIsEnabled = true;

		packet.DataLength = 8;

		xPortExtendedTransmition(&CAN_Local1, &packet);

		packet.ExtensionHeader.ServiceId = 334;
		xPortExtendedTransmition(&CAN_Local2, &packet);
	}
}
//==============================================================================
//initializations:

static TransferLayerAdapterT privateAdapter1;
static TransferLayerAdapterT privateAdapter2;
//==============================================================================
//initialization:

xResult TransferLayerComponentInit(void* parent)
{
	TransferLayerAdapterInitT adapterInit;
	adapterInit.Port = &CAN_Local1;
	TransferLayerAdapterInit(&LocalTransferLayer, &privateAdapter1, &adapterInit);

	adapterInit.Port = &CAN_Local2;
	TransferLayerAdapterInit(&ExternalTransferLayer, &privateAdapter2, &adapterInit);

	xTransferLayerInitT init;
	init.Parent = parent;
	init.Transfers = (void*)&CAN_LocalTransfersBuffer;
	init.TransfersCount = CAN_LOCAL_TRANSFER_BUFFER_SIZE;
	xTransferLayerInit(&LocalTransferLayer, &init);

	init.Transfers = (void*)&CAN_ExternalTransfersBuffer;
	init.TransfersCount = CAN_EXTERNAL_TRANSFER_BUFFER_SIZE;
	xTransferLayerInit(&ExternalTransferLayer, &init);

	return xResultAccept;
}
//==============================================================================
