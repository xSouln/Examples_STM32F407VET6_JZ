
//==============================================================================
//includes:

#include "Net-Component.h"
#include "Components.h"

#if NET_TARGET_LAYOUT == NET_LWIP_LAYOUT

#include "Adapters/LWIP/LWIP-Net-Adapter.h"
#include "Adapters/LWIP/LWIP-NetPort-Adapter.h"

#elif NET_TARGET_LAYOUT == NET_FREERTOS_LAYOUT

#include "Adapters/FreeRTOS-Plus-TCP/Net-Adapter.h"
#include "Adapters/FreeRTOS-Plus-TCP/NetPort-Adapter.h"

#endif
//==============================================================================
//defines:


//==============================================================================
//import:

#if NET_TARGET_LAYOUT == NET_LWIP_LAYOUT

extern struct netif gnetif;

#endif
//==============================================================================
//variables:

static uint8_t private_rx_operation_buffer[NET_RX_OPERATION_BUFFER_SIZE] NET_RX_OPERATION_BUFFER_MEM_SECTION;
static uint8_t private_rx_buffer[NET_RX_BUFFER_SIZE] NET_RX_BUFFER_MEM_SECTION;
static uint8_t private_tx_buffer[NET_RX_BUFFER_SIZE] NET_TX_BUFFER_MEM_SECTION;

static TaskHandle_t taskHandle;
static StaticTask_t taskBuffer;
static StackType_t taskStack[NET_TASK_STACK_SIZE] NET_COMPONENT_MAIN_TASK_STACK_SECTION;

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

xNetT Net NET_MEM_SECTION = { 0 };
xPortT NetPort NET_PORT_MEM_SECTION = { 0 };
//==============================================================================
//functions:

static void privateEventListener(ObjectBaseT* object, int selector, uint32_t description, void* arg)
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
				//xNetClose(&Socket);
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
static void privateTask(void* arg)
{
	char* result;

	while (true)
	{
		//vTaskDelay(pdMS_TO_TICKS(10));

		if (Net.PhyIsConnecnted
			&& ListenSocket.State == xNetSocketListen
			&& Socket.State == xNetSocketIdle)
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

		xPortHandler(&NetPort);

		RTOS_NetTcpServerTaskStackWaterMark = uxTaskGetStackHighWaterMark(NULL);
	}
}
//------------------------------------------------------------------------------
void NetComponentHandler()
{
	char* result;

	xNetHandler(&Net);
	xNetSocketHandler(&ListenSocket);

	if (Net.PhyIsConnecnted && Net.DHCP_Complite)
	{
		if (ListenSocket.State != xNetSocketListen)
		{
			if (xNetInitTcpSocket(&Net, &ListenSocket) == xResultAccept)
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

	//vTaskDelay(pdMS_TO_TICKS(33));
}
//==============================================================================
//initializations:

static NetAdapterT privateNetAdapter;
static NetPortAdapterT privateNetPortAdapter;
//==============================================================================
//initialization:

static xNetEventSubscriberT privateNetEventSubscriber =
{
	.EventListener = (void*)privateEventListener
};
//------------------------------------------------------------------------------

xResult NetComponentInit(void* parent)
{
#if NET_TARGET_LAYOUT == NET_LWIP_LAYOUT

	NetAdapterInitT adapterInit =
	{
		.gnetif = &gnetif
	};

#elif NET_TARGET_LAYOUT == NET_FREERTOS_LAYOUT

	NetAdapterInitT adapterInit;

#endif

	NetAdapterInit(&Net, &privateNetAdapter, &adapterInit);

	xNetInitT init =
	{
		.Parent = parent,
	};

	xNetInit(&Net, &init);
	xNetAddEventListener(&Net, &privateNetEventSubscriber);

	NetPortAdapterInitT netPortInit =
	{
		.RxOperationBuffer = private_rx_operation_buffer,
		.RxOperationBufferSize = sizeof(private_rx_operation_buffer),

		.RxBuffer = private_rx_buffer,
		.RxBufferSize = sizeof(private_rx_buffer),

		.TxBuffer = private_tx_buffer,
		.TxBufferSize = sizeof(private_tx_buffer)
	};

	NetPortAdapterInit(&NetPort, &privateNetPortAdapter, &netPortInit);

	xPortInitT portInit =
	{
		.Parent = parent,
		.EventListener = privateEventListener
	};

	xPortInit(&NetPort, &portInit);

	xPortSetBinding(&NetPort, &Socket);

	taskHandle =
			xTaskCreateStatic(privateTask, // Function that implements the task.
								"net task", // Text name for the task.
								NET_TASK_STACK_SIZE, // Number of indexes in the xStack array.
								NULL, // Parameter passed into the task.
								osPriorityNormal, // Priority at which the task is created.
								taskStack, // Array to use as the task's stack.
								&taskBuffer);

	return xResultAccept;
}
//==============================================================================
