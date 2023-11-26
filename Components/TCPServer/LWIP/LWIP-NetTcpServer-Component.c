//==============================================================================
//includes:

#include "Components.h"

#include "LWIP-NetTcpServer-Component.h"
#include "Adapters/LWIP-Net-Adapter.h"
#include "Adapters/LWIP-NetPort-Adapter.h"
//==============================================================================
//defines:

#define TASK_STACK_SIZE 0x200
#define RX_OPERATION_BUFFER_SIZE 0x200
#define RX_BUFFER_SIZE 0x200
#define TX_BUFFER_SIZE 0x400
//==============================================================================
//import:

extern struct netif gnetif;
//==============================================================================
//variables:

static uint8_t private_rx_operation_buffer[RX_OPERATION_BUFFER_SIZE];
static uint8_t private_rx_buffer[RX_BUFFER_SIZE];
static uint8_t private_tx_buffer[RX_BUFFER_SIZE];

static TaskHandle_t task_handle;
static StaticTask_t task_buffer;
static StackType_t taskStack[TASK_STACK_SIZE];

xNetSocketT ListenSocket =
{
	.Port = 5000,
	.Address.Value = 0,
	.Handle = (void*)-1
};

xNetSocketT Socket =
{
	.Handle = (void*)-1
};

int RTOS_NetTcpServerTaskStackWaterMark;

xNetT LWIP_Net;
xPortT ServerPort;
//==============================================================================
//functions:

static void PrivateEventListener(ObjectBaseT* object, int selector, void* arg)
{
	if (object->Description->ObjectId == xPORT_OBJECT_ID)
	{
		xPortT* port = (xPortT*)object;

		switch (selector)
		{
			case xPortObjectEventRxFoundEndLine:
			{
				TerminalReceiveData(port, arg);
			}
			break;

			case xPortObjectEventRxBufferIsFull:
			{
				TerminalReceiveData(port, arg);
			}
			break;

			default : return;
		}
	}
	else if (object->Description->ObjectId == xNET_OBJECT_ID)
	{
		xNetT* net = (xNetT*)object;

		switch(selector)
		{
			case xNetEventPhyConnected:
			{
				xNetDHCP_Start(net, 5000);
				break;
			}

			case xNetEventPhyDisconnected:
			{
				xNetClose(&ListenSocket);
				xNetClose(&Socket);
				break;
			}

			case xNetEventDHCP_Complite:
			{
				xNetSNTP_Start(net);
				break;
			}
		}
	}
}
//------------------------------------------------------------------------------
static void Task(void* arg)
{
	char* result;

	while (true)
	{
		//vTaskDelay(pdMS_TO_TICKS(10));

		if (ListenSocket.State == xNetSocketListen && Socket.State == xNetSocketIdle)
		{
			if (xNetAccept(&ListenSocket, &Socket) == xResultAccept)
			{
				result = "xNetAccept: xResultAccept\r";
				xPortStartTransmission(&SerialPort);
				xPortTransmitString(&SerialPort, result);
				xPortEndTransmission(&SerialPort);
			}
			else
			{
				result = "xNetAccept: xResultError\r";
			}
		}

		xPortHandler(&ServerPort);

		RTOS_NetTcpServerTaskStackWaterMark = uxTaskGetStackHighWaterMark(NULL);
	}
}
//------------------------------------------------------------------------------
void LWIP_NetTcpServerComponentHandler()
{
	char* result;

	xNetHandler(&LWIP_Net);
	xNetSocketHandler(&ListenSocket);

	if (LWIP_Net.PhyIsConnecnted && LWIP_Net.DHCP_Complite)
	{
		if (ListenSocket.State != xNetSocketListen)
		{
			if (xNetInitTcpSocket(&LWIP_Net, &ListenSocket) == xResultAccept)
			{
				result = "xNetInitTcpSocket: xResultAccept\r";
			}
			else
			{
				result = "xNetInitTcpSocket: xResultError\r";
			}

			xPortStartTransmission(&SerialPort);
			xPortTransmitString(&SerialPort, result);
			xPortEndTransmission(&SerialPort);

			if (xNetListen(&ListenSocket, 2) == xResultAccept)
			{
				result = "xNetListen: xResultAccept\r";
			}
			else
			{
				result = "xNetListen: xResultError\r";
			}

			xPortStartTransmission(&SerialPort);

			xPortTransmitString(&SerialPort, result);
			xPortEndTransmission(&SerialPort);

		}
	}

	//xPortHandler(&ServerPort);
}
//==============================================================================
//initializations:

LWIP_NetAdapterT LWIP_NetAdapter = { 0 };
LWIP_NetPortAdapterT LWIP_NetPortAdapter = { 0 };
//==============================================================================
//initialization:

static xNetEventSubscriberT privateNetEventSubscriber =
{
	.EventListener = (void*)PrivateEventListener
};
//------------------------------------------------------------------------------

xResult LWIP_NetTcpServerComponentInit(void* parent)
{
	LWIP_NetAdapterInitT adapterInit =
	{
		.gnetif = &gnetif
	};

	xNetInitT init =
	{
		.Parent = parent,

		.AdapterInit =
		{
			.Adapter = &LWIP_NetAdapter,
			.Init = &adapterInit,
			.Initializer = LWIP_NetAdapterInit
		}
	};
	xNetInit(&LWIP_Net, &init);
	xNetAddEventListener(&LWIP_Net, &privateNetEventSubscriber);

	LWIP_NetPortAdapterInitT netPortInit =
	{
		.RxOperationBuffer = private_rx_operation_buffer,
		.RxOperationBufferSize = sizeof(private_rx_operation_buffer),

		.RxBuffer = private_rx_buffer,
		.RxBufferSize = sizeof(private_rx_buffer),

		.TxBuffer = private_tx_buffer,
		.TxBufferSize = sizeof(private_tx_buffer)
	};

	xPortInitT port_init =
	{
		.Parent = parent,
		.EventListener = PrivateEventListener,

		.AdapterInit =
		{
			.Adapter = &LWIP_NetPortAdapter,
			.Init = &netPortInit,
		},
		.AdapterInitializer = LWIP_NetPortAdapterInit
	};
	xPortInit(&ServerPort, &port_init);

	xPortSetBinding(&ServerPort, &Socket);

	task_handle =
			xTaskCreateStatic(Task, // Function that implements the task.
								"Tcp server task", // Text name for the task.
								TASK_STACK_SIZE, // Number of indexes in the xStack array.
								NULL, // Parameter passed into the task.
								osPriorityNormal, // Priority at which the task is created.
								taskStack, // Array to use as the task's stack.
								&task_buffer);

	return xResultAccept;
}
//==============================================================================
