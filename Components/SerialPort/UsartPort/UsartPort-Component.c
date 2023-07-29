//==============================================================================
//includes:

#include "Components.h"
#include "Adapters/UsartPort-Adapter.h"
//==============================================================================
//defines:

#define UASRT_PORT_BUFFERS_INIT(name)\
static uint8_t name##_RX_CIRCLE_BUF[name##_RX_CIRCLE_BUF_SIZE_MASK + 1];\
static uint8_t name##_RX_OBJECT_BUF[name##_RX_OBJECT_BUF_SIZE];\
static uint8_t name##_TX_CIRCLE_BUF[name##_TX_CIRCLE_BUF_SIZE_MASK + 1]
//------------------------------------------------------------------------------
#define UASRT_PORT_ADAPTER_INIT(name, adapterInit)\
adapterInit.Usart = (REG_UART_T*)name##_REG;\
adapterInit.RxDMA = &name##_RX_DMA;\
adapterInit.RxBuffer = name##_RX_CIRCLE_BUF;\
adapterInit.RxBufferSizeMask = name##_RX_CIRCLE_BUF_SIZE_MASK;\
adapterInit.RxResponseBuffer = name##_RX_OBJECT_BUF;\
adapterInit.RxResponseBufferSize = name##_RX_OBJECT_BUF_SIZE;\
adapterInit.TxBuffer = name##_TX_CIRCLE_BUF;\
adapterInit.TxBufferSizeMask = name##_TX_CIRCLE_BUF_SIZE_MASK
//------------------------------------------------------------------------------
#define UASRT_PORT_ADAPTER_STRUCT_INIT(name)\
{\
	.Usart = (REG_UART_T*)name##_REG,\
	.RxDMA = &name##_RX_DMA,\
	.RxBuffer = name##_RX_CIRCLE_BUF,\
	.RxBufferSizeMask = name##_RX_CIRCLE_BUF_SIZE_MASK,\
	.RxResponseBuffer = name##_RX_OBJECT_BUF,\
	.RxResponseBufferSize = name##_RX_OBJECT_BUF_SIZE,\
	.TxBuffer = name##_TX_CIRCLE_BUF,\
	.TxBufferSizeMask = name##_TX_CIRCLE_BUF_SIZE_MASK\
}
//==============================================================================
//types:


//==============================================================================
//variables:

#ifdef SERIAL1_ENABLE
UASRT_PORT_BUFFERS_INIT(SERIAL1);
#endif

#ifdef SERIAL2_ENABLE
UASRT_PORT_BUFFERS_INIT(SERIAL2);
#endif

#ifdef SERIAL3_ENABLE
UASRT_PORT_BUFFERS_INIT(SERIAL3);
#endif

#ifdef SERIAL4_ENABLE
UASRT_PORT_BUFFERS_INIT(SERIAL4);
#endif

#ifdef SERIAL5_ENABLE
UASRT_PORT_BUFFERS_INIT(SERIAL5);
#endif

static const UsartPortAdapterInitT adapterInits[SERIAL_PORTS_COUNT] =
{
#ifdef SERIAL1_ENABLE
	UASRT_PORT_ADAPTER_STRUCT_INIT(SERIAL1),
#endif

#ifdef SERIAL2_ENABLE
	UASRT_PORT_ADAPTER_STRUCT_INIT(SERIAL2),
#endif

#ifdef SERIAL3_ENABLE
	UASRT_PORT_ADAPTER_STRUCT_INIT(SERIAL3),
#endif

#ifdef SERIAL4_ENABLE
	UASRT_PORT_ADAPTER_STRUCT_INIT(SERIAL4),
#endif

#ifdef SERIAL5_ENABLE
	UASRT_PORT_ADAPTER_STRUCT_INIT(SERIAL5),
#endif
};

xPortT SerialPorts[SERIAL_PORTS_COUNT];
//==============================================================================
//import:


//==============================================================================
//functions:

static void EventListener(xPortT* port, xPortObjectEventSelector selector, void* arg)
{
	switch((int)selector)
	{
		case xPortObjectEventRxFoundEndLine:
			TerminalReceiveData(port,
								((xPortEventDataPacketArgT*)arg)->Data,
								((xPortEventDataPacketArgT*)arg)->Size);
			break;

		case xPortObjectEventRxBufferIsFull:
			TerminalReceiveData(port,
								((xPortEventDataPacketArgT*)arg)->Data,
								((xPortEventDataPacketArgT*)arg)->Size);
			break;

		default: break;
	}
}
//------------------------------------------------------------------------------

void UsartPortComponentIRQ()
{

}
//------------------------------------------------------------------------------
//component functions:
/**
 * @brief main handler
 */
void UsartPortComponentHandler()
{
	for (uint8_t i = 0; i < SERIAL_PORTS_COUNT; i++)
	{
		xPortHandler(&SerialPorts[i]);
	}
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
void UsartPortComponentTimeSynchronization()
{

}

//==============================================================================
//initialization:

static UsartPortAdapterT PrivateUsartPortAdapter[SERIAL_PORTS_COUNT];
//==============================================================================
//component initialization:

xResult UsartPortComponentInit(void* parent)
{
	xPortInitT init =
	{
		.Parent = parent,
		.EventListener = (xObjectEventListenerT)EventListener,

		.AdapterInit =
		{
			.Adapter = PrivateUsartPortAdapter,
			.Initializer = UsartPortAdapterInit
		}
	};

	for (int i = 0; i < SERIAL_PORTS_COUNT; i++)
	{
		init.AdapterInit.Init = &adapterInits[i];
		xPortInit(&SerialPorts[i], &init);
	}
  
	return 0;
}
//==============================================================================

