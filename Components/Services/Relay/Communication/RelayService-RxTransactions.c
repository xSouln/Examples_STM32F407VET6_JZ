//==============================================================================
//includes:

#include "RelayService-RxTransactions.h"
#include "Abstractions/xDevice/xDevice.h"
#include "Abstractions/xSystem/xSystem.h"
#include "../RelayService.h"
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

static xResult RelayServiceGetPinsState(xRxRequestManagerT* manager, RequestGetTemperatureT* request)
{


	return xResultAccept;
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
		.Id = RelayServiceRequestGetPinsState,
		.Action = (xRxTransactionActionT)RelayServiceGetPinsState
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
	.Address = { .UID = RELAY_SERVICE_UID }
};
//------------------------------------------------------------------------------
/**
 * @brief определение возможных запросов к компоненту
 */
const xRxRequestT RelayServiceRxRequests[] =
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
