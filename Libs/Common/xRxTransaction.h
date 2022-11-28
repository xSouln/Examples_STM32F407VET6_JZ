//==============================================================================
#ifndef X_RX_TRANSACTION_H
#define X_RX_TRANSACTION_H
//==============================================================================
//includes:

#include "xRxRequest.h"
#include "xRx.h"
#include "xDataBuffer.h"
#include "Adapters/xRxTransactionTransferAdapter.h"
//==============================================================================
//packet types:

typedef union
{
	struct
	{
		uint32_t StartKey : 8; //default '#'
		uint32_t Description : 16; //description of the purpose of the package - request, response, event, etc.
		uint32_t EndKey : 8; //default ':'
	};
	
	uint32_t Value;
	
} PacketIdentificatorT; //aligned for uint32_t
//------------------------------------------------------------------------------

typedef struct
{
	PacketIdentificatorT Identificator; //identifier for recognizing the package
	uint32_t DeviceKey; //unique key of the device, module. 0 - system commands
	
} PacketHeaderT; //aligned for uint32_t
//------------------------------------------------------------------------------

typedef struct
{
	uint32_t RequestId; //generated key - when receiving a response, must match the request key
	
	uint16_t ActionKey; //action(command) key
	uint16_t ContentSize; //size of nested data after packet info
	
} PacketInfoT; //aligned for uint32_t
//------------------------------------------------------------------------------

typedef struct
{
	PacketHeaderT Header; // format: [#][Description][:][DeviceKey]
	PacketInfoT Info; // format: [RequestId][ActionKey][ContentSize]
	//uint8_t Content[Info.ContentSize]
	//uint8_t end packet identificator - default('\r')
	
} PacketT; //array: [#][Description][:][DeviceKey][RequestId][ActionKey][ContentSize][uint8_t Content[ContentSize]][\r]
//==============================================================================
//defines:

#define PACKET_HEADER_IDENTIFICATOR_MASK 0xFF0000FFU
#define PACKET_HEADER_IDENTIFICATOR_START_KEY '#'
#define PACKET_HEADER_IDENTIFICATOR_END_KEY ':'
#define PACKET_HEADER_IDENTIFICATOR 0x2300003AU // format: "#{Description}:"
//------------------------------------------------------------------------------

#define PACKET_HEADER_DESCRIPTION_MASK 0x00FFFF00U // format: "#{Description}:"
#define PACKET_HEADER_DESCRIPTION_REQUEST 0x5251U // "RQ"
#define PACKET_HEADER_DESCRIPTION_RESPONSE 0x5253U // "RS"
#define PACKET_HEADER_DESCRIPTION_EVENT 0x4556U // "EV"
#define PACKET_HEADER_DESCRIPTION_ERROR 0x4552U // "EV"
//------------------------------------------------------------------------------

#define PACKET_END_IDENTIFICATOR "\r" // default
//------------------------------------------------------------------------------

#define TRANSACTION_REQUEST_IDENTIFICATOR (PACKET_HEADER_IDENTIFICATOR | PACKET_HEADER_DESCRIPTION_REQUEST << 8)
#define TRANSACTION_RESPONSE_IDENTIFICATOR (PACKET_HEADER_IDENTIFICATOR | PACKET_HEADER_DESCRIPTION_RESPONSE << 8)
#define TRANSACTION_EVENT_IDENTIFICATOR (PACKET_HEADER_IDENTIFICATOR | PACKET_HEADER_DESCRIPTION_EVENT << 8)
#define TRANSACTION_ERROR_IDENTIFICATOR (PACKET_HEADER_IDENTIFICATOR | PACKET_HEADER_DESCRIPTION_ERROR << 8)

#define TRANSACTION_END_IDENTIFICATOR "\r"
//==============================================================================
//types:

typedef void (*xRxTransactionAction)(xRxRequestManagerT* manager,
										void* request,
										uint16_t request_size);
//------------------------------------------------------------------------------

typedef struct
{
  uint16_t Description;
  uint16_t Id;
  xRxTransactionAction Action;
  
} xRxTransactionT;
//==============================================================================
//functions:

xRxTransactionT* xRxTransactionIdentify(xRxTransactionT* transaction, uint16_t key);

xResult xRxTransactionTransmitEvent(xTxT* tx, uint32_t device_id, uint16_t transaction, void* data, uint16_t data_size);
xResult xRxTransactionRequestReceiver(xRxRequestManagerT* manager, uint8_t* object, uint16_t size);
xResult xRxTransactionError(xTxT* tx, PacketT* request_header, void* data, uint16_t data_size);
//==============================================================================
#endif //X_RX_TRANSACTION_H
