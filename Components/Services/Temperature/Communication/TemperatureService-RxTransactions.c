//==============================================================================
//includes:

#include "TemperatureService-RxTransactions.h"
#include "Abstractions/xDevice/xDevice.h"
#include "Abstractions/xSystem/xSystem.h"
#include "../TemperatureService.h"
//==============================================================================
//types:

typedef struct
{
	uint32_t DeviceId;
	uint32_t ServiceId;

} RequestGetTemperatureT;
//==============================================================================
//variables:


//==============================================================================
//functions:


//==============================================================================
//GET:
/// @defgroup xDeviceControlTransactionsGet xDeviceControl transacrions GET
/// @brief функции приема запросов на получение параметров компонента xDeviceControlT
/// @{

static xResult TemperatureServiceGetTemperature(xRxRequestManagerT* manager, RequestGetTemperatureT* request)
{
	xDeviceT* device = manager->Object;

	if (device->Id != request->DeviceId)
	{
		return xResultError;
	}

	xServiceListElementT* element = xListStartEnumeration((xListT*)&device->Services);
	xResult result = xResultError;

	while (element)
	{
		xServiceT* service = element->Value;

		if (service->Id == request->ServiceId)
		{
			float temperature;
			result = xServiceRequestListener(service, TemperatureServiceRequestGetTemperature, &temperature);

			if (result == xResultAccept)
			{
				xDataBufferAdd(manager->ResponseBuffer, &temperature, sizeof(temperature));
			}

			break;
		}

		element = element->Next;
	}

	xListStopEnumeration((xListT*)&device->Services);

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
