//==============================================================================
//includes:

#include "CAN_Example-Component.h"
#include "can.h"
#include "Adapters/CAN_Port-Adapter.h"
#include "Abstractions/xSystem/xSystem.h"
#include "Components.h"
//==============================================================================
//defines:

#define TASK_STACK_SIZE 0x200

#define TX_CIRCLE_BUF_SIZE_MASK 0x7f
#define RX_CIRCLE_BUF_SIZE_MASK 0x3f
#define RX_OBJECT_BUF_SIZE 125
//==============================================================================
//import:

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;
//==============================================================================
//variables:

static TaskHandle_t taskHandle;
static StaticTask_t taskBuffer;
static StackType_t taskStack[TASK_STACK_SIZE];

CAN_RegT* RegisterCAN1;
CAN_RegT* RegisterCAN2;

CAN_PortAdapterT CAN1_PortAdapter;
CAN_PortAdapterT CAN2_PortAdapter;

uint8_t CAN1_TxCircleBuffer[TX_CIRCLE_BUF_SIZE_MASK + 1];
uint8_t CAN1_RxCircleBuffer[RX_CIRCLE_BUF_SIZE_MASK + 1];
uint8_t CAN1_RxObjectBuffer[RX_OBJECT_BUF_SIZE];

uint8_t CAN2_TxCircleBuffer[TX_CIRCLE_BUF_SIZE_MASK + 1];
uint8_t CAN2_RxCircleBuffer[RX_CIRCLE_BUF_SIZE_MASK + 1];
uint8_t CAN2_RxObjectBuffer[RX_OBJECT_BUF_SIZE];

xPortT CAN1_Port;
xPortT CAN2_Port;

static uint32_t timeStamp;
//==============================================================================
//functions:

static void PrivateEventListener(ObjectBaseT* object, int selector, void* arg)
{
	RxDataPacketT* dataPacket = arg;
	xPortT* port = (void*)object;

	/*if (selector == xPortObjectEventRxFoundEndLine || selector == xPortObjectEventRxBufferIsFull)
	{
		TerminalReceiveData(port, arg);
	}*/

	if (port == &CAN1_Port)
	{
		xPortStartTransmission(&SerialPort);
		xPortTransmitString(&SerialPort, "CAN1 data: ");
		xPortTransmitData(&SerialPort, dataPacket->Data, dataPacket->Size);
		xPortTransmitString(&SerialPort, "\r");
		xPortEndTransmission(&SerialPort);
	}
	else if (port == &CAN2_Port)
	{
		xPortStartTransmission(&SerialPort);
		xPortTransmitString(&SerialPort, "CAN2 data: ");
		xPortTransmitData(&SerialPort, dataPacket->Data, dataPacket->Size);
		xPortTransmitString(&SerialPort, "\r");
		xPortEndTransmission(&SerialPort);
	}

	xPortClearRxBuffer(port);
}
//------------------------------------------------------------------------------
static void privateTask(void* arg)
{
	CAN_FilterTypeDef filterConfig;
	filterConfig.FilterBank = 0;
	filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	filterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	filterConfig.FilterIdHigh = 0x0000;
	filterConfig.FilterIdLow = 0x0000;
	filterConfig.FilterMaskIdHigh = 0x0000;
	filterConfig.FilterMaskIdLow = 0x0000;
	filterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	filterConfig.FilterActivation = ENABLE;
	filterConfig.SlaveStartFilterBank = 0;

	CAN_PortAdapterInitT portAdapterInit;
	portAdapterInit.CAN = &hcan1;
	portAdapterInit.CAN_Number = xCAN1;
	portAdapterInit.TxBuffer = CAN1_TxCircleBuffer;
	portAdapterInit.RxBuffer = CAN1_RxCircleBuffer;
	portAdapterInit.RxResponseBuffer = CAN1_RxObjectBuffer;
	portAdapterInit.FilterConfig = &filterConfig;
	portAdapterInit.TxBufferSizeMask = TX_CIRCLE_BUF_SIZE_MASK;
	portAdapterInit.RxBufferSizeMask = RX_CIRCLE_BUF_SIZE_MASK;
	portAdapterInit.RxResponseBufferSize = RX_OBJECT_BUF_SIZE;

	xPortInitT portInit;
	portInit.Parent = taskHandle;
	portInit.AdapterInit.Adapter = &CAN1_PortAdapter;
	portInit.AdapterInit.Init = &portAdapterInit;
	portInit.AdapterInitializer = CAN_PortAdapterInit;
	portInit.EventListener = PrivateEventListener;
	xPortInit(&CAN1_Port, &portInit);

	filterConfig.FilterBank = 1;
	portAdapterInit.CAN = &hcan2;
	portAdapterInit.CAN_Number = xCAN2;
	portAdapterInit.TxBuffer = CAN2_TxCircleBuffer;
	portAdapterInit.RxBuffer = CAN2_RxCircleBuffer;
	portAdapterInit.RxResponseBuffer = CAN2_RxObjectBuffer;
	portInit.AdapterInit.Adapter = &CAN2_PortAdapter;
	xPortInit(&CAN2_Port, &portInit);

	#define TX_DATA "dvmoqwhgolSDJVIOSionpmpcoKQFEWJGUBETBVNAIMEO,CSS{lcspJHBRYEVIU5GHI	PockoZMCL{LlOJFUWRBVETQIUVNOMPOM,Cp[ewpomgo	iwrngojMV\r"

	while (true)
	{
		xPortDirectlyHandler(CAN1_Port);
		xPortDirectlyHandler(CAN2_Port);

		if (xSystemGetTime(privateTask) - timeStamp > 1000)
		{
			timeStamp = xSystemGetTime(privateTask);

			xPortTransmitString(&CAN1_Port, TX_DATA);
			xPortTransmitString(&CAN2_Port, TX_DATA);
		}
	}
}
//------------------------------------------------------------------------------
void CAN_ExampleComponentHandler()
{

}
//------------------------------------------------------------------------------
void CAN_ExampleComponentTimeSynchronization()
{

}
//==============================================================================
//initializations:


//==============================================================================
//initialization:

xResult CAN_ExampleComponentInit(void* parent)
{
	RegisterCAN1 = rCAN1;
	RegisterCAN2 = rCAN2;

	taskHandle =
			xTaskCreateStatic(privateTask, // Function that implements the task.
								"CAN example task", // Text name for the task.
								TASK_STACK_SIZE, // Number of indexes in the xStack array.
								NULL, // Parameter passed into the task.
								osPriorityNormal, // Priority at which the task is created.
								taskStack, // Array to use as the task's stack.
								&taskBuffer);

	return xResultAccept;
}
//==============================================================================
