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

static xTransferT privateTransfer;
static xTransferT* privateMasterTransfer;

volatile static struct
{
	uint32_t CAN_Local1NoiseIsEnable : 1;
	uint32_t CAN_Local2NoiseIsEnable : 1;

} privateFlags;
//==============================================================================
//functions:

static void transferComplite(xTransferLayerT* layer, xTransferT* transfer, int selector, void* arg)
{
	transfer->State = xTransferStateIdle;
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

		privateMasterTransfer = &privateTransfer;
		if (privateMasterTransfer->State == xTransferStateIdle)
		{
			privateMasterTransfer->Holder = HostDevice.Services.Head->Value;
			privateMasterTransfer->Id = TemperatureService3.Base.Id;
			//privateMasterTransfer.Base.Type = xTransferTypeTransmite;
			//privateMasterTransfer.Base.ValidationIsEnabled = true;
			privateMasterTransfer->MasterModeIsEnabled = true;

			privateMasterTransfer->TimeOut = 1000;
			privateMasterTransfer->EventListener = transferComplite;
			privateMasterTransfer->TransmittingAttempts = 1;

			if (privateMasterTransfer->Type == xTransferTypeTransmite)
			{
				privateMasterTransfer->Data = (uint8_t*)transferTxData;
				privateMasterTransfer->DataLength = sizeof_str(transferTxData);
			}
			else if (privateMasterTransfer->Type == xTransferTypeReceive)
			{
				memset(transferRxData, 0, sizeof(transferRxData));
				privateMasterTransfer->Data = (uint8_t*)transferRxData;
				privateMasterTransfer->DataLength = sizeof(transferRxData);
			}

			xTransferLayerAdd(&LocalTransferLayer, privateMasterTransfer);
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

		if (privateFlags.CAN_Local1NoiseIsEnable)
		{
			xPortExtendedTransmition(&CAN_Local1, &packet);
		}

		packet.ExtensionHeader.ServiceId = 334;

		if (privateFlags.CAN_Local2NoiseIsEnable)
		{
			xPortExtendedTransmition(&CAN_Local2, &packet);
		}
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
