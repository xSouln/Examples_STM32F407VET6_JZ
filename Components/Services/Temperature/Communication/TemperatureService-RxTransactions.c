//==============================================================================
//includes:

#include "TemperatureService-RxTransactions.h"
#include "Abstractions/xDevice/xDevice.h"
#include "Abstractions/xSystem/xSystem.h"
#include "Common/xMemory.h"
#include "../TemperatureService.h"
//==============================================================================
//types:

typedef struct
{
	xPortT* Port;
	PacketInfoT PacketInfo;

} AsyncRequestT;
//------------------------------------------------------------------------------

typedef struct
{
	uint32_t DeviceId;
	uint32_t ServiceId;

} RequestGetTemperatureT;
//------------------------------------------------------------------------------

typedef struct
{
	xRxTransactionContentT Content;

	xPortT* Port;

} xAsyncRequestContentT;
//==============================================================================
//variables:


//==============================================================================
//functions:


//==============================================================================
//GET:
/// @defgroup xDeviceControlTransactionsGet xDeviceControl transacrions GET
/// @brief функции приема запросов на получение параметров компонента xDeviceControlT
/// @{

static void privateAsyncCallback(xIAsyncResultT* result, ...)
{
	xAsyncRequestContentT* content = result->Content;

	if (result->OperationResult == xResultAccept)
	{
		xRxTransactionAsyncResponse(content->Port, &content->Content, result->Result, result->ResultSize);
	}

	xMemoryFree(content);
}
//------------------------------------------------------------------------------
static xResult privatePrepareAsyncRequest(xRxRequestManagerT* manager, xIAsyncRequestT* asyncRequest, void* holder)
{
	xAsyncRequestContentT* content = xMemoryAllocate(1, sizeof(xAsyncRequestContentT));
	memcpy(&content->Content, manager->Content, sizeof(xRxTransactionContentT));
	content->Port = manager->Port;

	memset(asyncRequest, 0, sizeof(xIAsyncRequestT));
	asyncRequest->Holder = holder;
	asyncRequest->Callback = privateAsyncCallback;
	asyncRequest->Content = content;

	return xResultAccept;
}
//------------------------------------------------------------------------------

static xResult TemperatureServiceGetTemperature(xRxRequestManagerT* manager, RequestGetTemperatureT* request)
{
	xResult result = xResultError;
	xDeviceT* device = xDeviceGetDeviceById(manager->Object, request->DeviceId);

	if (device)
	{
		xServiceT* service = xDeviceGetServiceById(device, request->ServiceId);

		if (service)
		{
			xIAsyncRequestT asyncRequest;
			privatePrepareAsyncRequest(manager, &asyncRequest, NULL);

			xServiceRequestListener(service, TemperatureServiceRequestGetTemperature, &asyncRequest);
		}
	}

	return result;
}

/// @}
//==============================================================================
//SET:
/// @defgroup xDeviceControlTransactionsSet xDeviceControl transacrions SET
/// @brief функции приема запросов управления компонентом xDeviceControlT
/// @{


/// @}
//==============================================================================
//TRY:
/// @defgroup xDeviceControlTransactionsTry xDeviceControl transacrions TRY
/// @brief функции приема запросов управления компонентом xDeviceControlT
/// @{


/// @}
//==============================================================================
//initializations:

/**
 * @brief привязка функций к ActionKey PacketInfoT согласованных поддерживаемыми
 * опреациями xDeviceControlTransactions
 */
static const xRxTransactionT privateRxTransactions[] =
{
	//--------------------------------------------------------------------------
	//GET

	{
		.Id = TemperatureServiceRequestGetTemperature,
		.Action = (xRxTransactionActionT)TemperatureServiceGetTemperature
	},
	//----------------------------------------------------------------------------
	//SET


	//----------------------------------------------------------------------------
	//TRY


	//----------------------------------------------------------------------------
	//end of transactions marker
	{ .Id = -1 }
};

//------------------------------------------------------------------------------
/**
 * @brief определения заголовка сервиса для фильтрации сообщений
 */
static const PacketHeaderT privateTransactionRequestHeader =
{
	.Identifier = { .Value = TRANSACTION_REQUEST_IDENTIFIER },
	.Address = { .UID = TEMPERATURE_SERVICE_UID }
};
//------------------------------------------------------------------------------
/**
 * @brief определение возможных запросов к компоненту
 */
const xRxRequestT TemperatureServiceRxRequests[] =
{
	/*{
		.Header = (void*)&privateTransactionRequestHeader,
		.HeaderLength = sizeof(privateTransactionRequestHeader),
		.Action = (xRxRequestReceiverT)xDeviceControlPortBinding
	},*/

	{
		.Header = (void*)&privateTransactionRequestHeader,
		.HeaderLength = sizeof(privateTransactionRequestHeader),
		.Action = (xRxRequestReceiverT)xRxTransactionRequestReceiver,
		.Content = (void*)&privateRxTransactions,
		.IsLastElement = true
	}
};
//==============================================================================
