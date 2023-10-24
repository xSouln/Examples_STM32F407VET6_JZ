//==============================================================================
//includes:

#include "CAN_Local-Component.h"
#include "Components.h"
#if defined(STM32F1)
#include "Adapters/STM32F1xx/UsartPort-Adapter.h"
#elif defined(STM32F4)
#include "Adapters/STM32F4xx/CAN_LocalPort-Adapter.h"
#elif defined(STM32H7)
#include "Adapters/STM32H7xx/UsartPort-Adapter.h"
#endif
//==============================================================================
//defines:

#define CAN_PORT_BUFFERS_INIT(name)\
static CAN_LocalSegmentT name##_RX_CIRCLE_BUF[name##_RX_CIRCLE_BUF_SIZE_MASK + 1];\
static CAN_LocalSegmentT name##_TX_CIRCLE_BUF[name##_TX_CIRCLE_BUF_SIZE_MASK + 1]
//------------------------------------------------------------------------------
#define CAN_PORT_ADAPTER_INIT(name, adapterInit)\
adapterInit.CAN = name##_HANDLE;\
adapterInit.CAN_Number = name##_PORT_NUMBER;\
adapterInit.RxBuffer = name##_RX_CIRCLE_BUF;\
adapterInit.RxBufferSizeMask = name##_RX_CIRCLE_BUF_SIZE_MASK;\
adapterInit.TxBuffer = name##_TX_CIRCLE_BUF;\
adapterInit.TxBufferSizeMask = name##_TX_CIRCLE_BUF_SIZE_MASK
//------------------------------------------------------------------------------
#define CAN_PORT_ADAPTER_STRUCT_INIT(name)\
{\
	.CAN = &name##_HANDLE,\
	.CAN_Number = name##_PORT_NUMBER,\
	.FilterBank = name##_FILTER_BANK,\
	.FilterMode = name##_FILTER_MODE,\
	.FilterScale = name##_FILTER_SCALE,\
	.FilterFIFOAssignment = name##_FILTER_RX_FIFO,\
	.FilterIdHigh = name##_FILTER_ID_HIGH,\
	.FilterIdLow = name##_FILTER_ID_LOW,\
	.FilterMaskIdHigh = name##_FILTER_MASK_ID_HIGH,\
	.FilterMaskIdLow = name##_FILTER_MASK_ID_LOW,\
	.RxBuffer = name##_RX_CIRCLE_BUF,\
	.RxBufferSizeMask = name##_RX_CIRCLE_BUF_SIZE_MASK,\
	.TxBuffer = name##_TX_CIRCLE_BUF,\
	.TxBufferSizeMask = name##_TX_CIRCLE_BUF_SIZE_MASK\
}
//==============================================================================
//types:


//==============================================================================
//variables:

#ifdef CAN_LOCAL1_ENABLE
CAN_PORT_BUFFERS_INIT(CAN_LOCAL1);
#endif

#ifdef CAN_LOCAL2_ENABLE
CAN_PORT_BUFFERS_INIT(CAN_LOCAL2);
#endif

#ifdef CAN_LOCAL3_ENABLE
CAN_PORT_BUFFERS_INIT(CAN_LOCAL3);
#endif

#ifdef CAN_LOCAL4_ENABLE
CAN_PORT_BUFFERS_INIT(CAN_LOCAL4);
#endif

#ifdef CAN_LOCAL5_ENABLE
CAN_PORT_BUFFERS_INIT(CAN_LOCAL5);
#endif

#ifdef CAN_LOCAL6_ENABLE
CAN_PORT_BUFFERS_INIT(CAN_LOCAL6);
#endif

static const CAN_LocalPortAdapterInitT adapterInits[CAN_LOCAL_PORTS_COUNT] =
{
#ifdef CAN_LOCAL1_ENABLE
	CAN_PORT_ADAPTER_STRUCT_INIT(CAN_LOCAL1),
#endif

#ifdef CAN_LOCAL2_ENABLE
	CAN_PORT_ADAPTER_STRUCT_INIT(CAN_LOCAL2),
#endif

#ifdef CAN_LOCAL3_ENABLE
	CAN_PORT_ADAPTER_STRUCT_INIT(CAN_LOCAL3),
#endif

#ifdef CAN_LOCAL4_ENABLE
	CAN_PORT_ADAPTER_STRUCT_INIT(CAN_LOCAL4),
#endif

#ifdef CAN_LOCAL5_ENABLE
	CAN_PORT_ADAPTER_STRUCT_INIT(CAN_LOCAL5),
#endif

#ifdef CAN_LOCAL6_ENABLE
	CAN_PORT_ADAPTER_STRUCT_INIT(CAN_LOCAL6),
#endif
};

xPortT CAN_LocalPorts[CAN_LOCAL_PORTS_COUNT];
//==============================================================================
//import:


//==============================================================================
//functions:

static void EventListener(xPortT* port, xPortObjectEventSelector selector, void* arg)
{
	switch((int)selector)
	{
		default: break;
	}
}
//------------------------------------------------------------------------------
//component functions:
/**
 * @brief main handler
 */
void CAN_LocalComponentHandler()
{
	for (uint8_t i = 0; i < CAN_LOCAL_PORTS_COUNT; i++)
	{
		xPortHandler(&CAN_LocalPorts[i]);
	}
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
void CAN_LocalComponentTimeSynchronization()
{

}

//==============================================================================
//initialization:

static CAN_LocalPortAdapterT privatePortAdapters[CAN_LOCAL_PORTS_COUNT];
//==============================================================================
//component initialization:

xResult CAN_LocalComponentInit(void* parent)
{
	xPortInitT init =
	{
		.Parent = parent,
		.EventListener = (xObjectEventListenerT)EventListener,
		.AdapterInitializer = CAN_LocalPortAdapterInit
	};

	for (int i = 0; i < CAN_LOCAL_PORTS_COUNT; i++)
	{
		init.AdapterInit.Init = (void*)&adapterInits[i];
		init.AdapterInit.Adapter = &privatePortAdapters[i];
		xPortInit(&CAN_LocalPorts[i], &init);
	}
  
	return 0;
}
//==============================================================================

