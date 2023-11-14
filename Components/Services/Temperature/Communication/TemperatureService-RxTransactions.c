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

static void privateCallbackGetTemperature(xServiceAsyncRequestManagerT* manager)
{
	xAsyncRequestContentT* content = manager->Content;

	float result = *(float*)manager->Result;

	xRxTransactionAsyncResponse(content->Port, &content->Content, &result, sizeof(result));

	xMemoryFree(content);
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
			/*float temperature;
			result = xServiceRequestListener(service, TemperatureServiceRequestGetTemperature, &temperature);*/

			xAsyncRequestContentT* content = xMemoryAllocate(1, sizeof(xAsyncRequestContentT));
			memcpy(&content->Content, manager->Content, sizeof(xRxTransactionContentT));
			content->Port = manager->Port;

			xServiceAsyncRequestT asyncRequest = { 0 };
			asyncRequest.Callback = privateCallbackGetTemperature;
			asyncRequest.Content = content;

			xServiceRequestListener(service, TemperatureServiceRequestGetTemperature, &asyncRequest);

			/*if (result == xResultAccept)
			{
				xDataBufferAdd(manager->ResponseBuffer, &temperature, sizeof(temperature));
			}*/
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
