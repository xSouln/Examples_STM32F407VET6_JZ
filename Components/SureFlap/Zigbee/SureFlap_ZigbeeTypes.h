//==============================================================================
#ifndef _SUREFLAP_ZIGBEE_TYPES_H
#define _SUREFLAP_ZIGBEE_TYPES_H
//------------------------------------------------------------------------------
#include "SureFlap/SureFlap_ComponentConfig.h"
#ifdef SUREFLAP_COMPONENT_ENABLE
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components_Types.h"
#include "SureFlap_Config.h"
#include "Adapters/SureFlap_ZigbeeAdapterBase.h"
//==============================================================================
//defines:

// These #defines are used to create the Beacon Payload
#define SUREFLAP_BEACON_PAYLOAD_SUREFLAP_HUB                    0x7e
#define SUREFLAP_BEACON_PAYLOAD_HUB_SUPPORTS_THALAMUS           0x02
#define SUREFLAP_BEACON_PAYLOAD_HUB_DOES_NOT_SUPPORT_THALAMUS   0x01
#define SUREFLAP_BEACON_PAYLOAD_HUB_IS_SOLE_PAN_COORDINATOR     0x00

//Dummy register values to indicate special commands which cannot be split for transmission or need special handling
#define SUREFLAP_ZIGBEE_MOVEMENT_DUMMY_REGISTER 1024
//==============================================================================
//types:

typedef enum
{
	SUREFLAP_BEACON_PAYLOAD_PROTOCOL_ID,
	SUREFLAP_BEACON_PAYLOAD_VERSION,
	SUREFLAP_BEACON_PAYLOAD_SOLE_PAN_COORDINATOR,

} SureFlapZigbeeBeaconPayloadsIndex;
//------------------------------------------------------------------------------

// used to indicate where a pairing request came from
typedef enum
{
	PAIRING_REQUEST_SOURCE_UNKNOWN,
	PAIRING_REQUEST_SOURCE_SERVER,
	PAIRING_REQUEST_SOURCE_BUTTON,
	PAIRING_REQUEST_SOURCE_CLI,
	PAIRING_REQUEST_SOURCE_TIMEOUT,
	PAIRING_REQUEST_SOURCE_BEACON_REQUEST,

} SureFlapZigbeeParingRequestSources;
//------------------------------------------------------------------------------

typedef enum
{
	SUREFLAP_ZIGBEE_PACKET_UNKNOWN,
	SUREFLAP_ZIGBEE_PACKET_DATA,
	SUREFLAP_ZIGBEE_PACKET_DATA_ACK,
	SUREFLAP_ZIGBEE_PACKET_ACK,
	SUREFLAP_ZIGBEE_PACKET_BEACON,
	SUREFLAP_ZIGBEE_PACKET_PAIRING_REQUEST,
	SUREFLAP_ZIGBEE_PACKET_PAIRING_CONFIRM,
	SUREFLAP_ZIGBEE_PACKET_CHANNEL_HOP,
	SUREFLAP_ZIGBEE_PACKET_DEVICE_AWAKE,
	SUREFLAP_ZIGBEE_PACKET_DEVICE_TX, // 9
	SUREFLAP_ZIGBEE_PACKET_END_SESSION,
	SUREFLAP_ZIGBEE_PACKET_DETACH,
	SUREFLAP_ZIGBEE_PACKET_DEVICE_SLEEP,
	SUREFLAP_ZIGBEE_PACKET_DEVICE_P2P,
	SUREFLAP_ZIGBEE_PACKET_ENCRYPTION_KEY,
	SUREFLAP_ZIGBEE_PACKET_REPEATER_PING,
	SUREFLAP_ZIGBEE_PACKET_PING,
	SUREFLAP_ZIGBEE_PACKET_PING_R,
	SUREFLAP_ZIGBEE_PACKET_REFUSE_AWAKE,
	SUREFLAP_ZIGBEE_PACKET_DEVICE_STATUS, // 0x13 / 19
	SUREFLAP_ZIGBEE_PACKET_DEVICE_CONFIRM,  //20
	SUREFLAP_ZIGBEE_PACKET_DATA_SEGMENT,
	SUREFLAP_ZIGBEE_PACKET_BLOCKING_TEST,
	SUREFLAP_ZIGBEE_PACKET_DATA_ALT_ENCRYPTED,

} SureFlapZigbeePacketTypes;
//------------------------------------------------------------------------------
// These are COMMAND types for messages to/from Devices. Most are not used.
typedef enum
{
	SUREFLAP_ZIGBEE_COMMAND_INVALID =  0,
	//Messages to exchange register values
	SUREFLAP_ZIGBEE_COMMAND_GET_REG,          // 1
	SUREFLAP_ZIGBEE_COMMAND_SET_REG,          // 2
	SUREFLAP_ZIGBEE_COMMAND_GET_REG_R,        // 3
	SUREFLAP_ZIGBEE_COMMAND_SET_REG_R,        // 4
	//Messages for SPI Slave Module interaction
	SUREFLAP_ZIGBEE_COMMAND_GET_FIRMWARE_VERSION,   // 5
	SUREFLAP_ZIGBEE_COMMAND_GET_FIRMWARE_VERSION_R, // 6
	SUREFLAP_ZIGBEE_COMMAND_GET_HARDWARE_VERSION,   // 7
	SUREFLAP_ZIGBEE_COMMAND_GET_HARDWARE_VERSION_R,	// 8
	SUREFLAP_ZIGBEE_COMMAND_STATUS,                 // 9  // poll for status
	SUREFLAP_ZIGBEE_COMMAND_STATUS_R,               // 10
	//SPI Response message (holds error code)
	SUREFLAP_ZIGBEE_COMMAND_ACK,                    // 11
	//Error response message (holds error code)
	SUREFLAP_ZIGBEE_COMMAND_ERROR_REPORT,           // 12
	SUREFLAP_ZIGBEE_COMMAND_CLAIM_REGISTER_SET,     // 13
	SUREFLAP_ZIGBEE_COMMAND_FREE_REGISTER_SET,      // 14
	SUREFLAP_ZIGBEE_COMMAND_ADD_REGISTER_SET,       // 15
	SUREFLAP_ZIGBEE_COMMAND_SEND_REGISTER_SET,      // 16
	SUREFLAP_ZIGBEE_COMMAND_SEND_MSG,               // 17
	SUREFLAP_ZIGBEE_COMMAND_SEND_MSG_R,             // 18
	SUREFLAP_ZIGBEE_COMMAND_THALAMUS,              // 19
	SUREFLAP_ZIGBEE_COMMAND_THALAMUS_MULTIPLE,     // 20
	SUREFLAP_ZIGBEE_COMMAND_TEST_MSG,               // 21

	//Messages for RF control
	SUREFLAP_ZIGBEE_COMMAND_SET_PAIRING_NORMAL=32,  // 32
	SUREFLAP_ZIGBEE_COMMAND_SET_PAIRING_MANUAL,     // 33
	SUREFLAP_ZIGBEE_COMMAND_RESET_SURENET,          // 34
	SUREFLAP_ZIGBEE_COMMAND_UNPAIR_DEVICE,          // 35
	SUREFLAP_ZIGBEE_COMMAND_CHECK_DATA,             // 36
	SUREFLAP_ZIGBEE_COMMAND_PROGRAMMER_DATA,        // 37 return values needed by programmer, currently just MAC address
	SUREFLAP_ZIGBEE_COMMAND_PROGRAMMER_DATA_R       // 38

} SureFlapZigbeeCommands;
//------------------------------------------------------------------------------
// Not used in Hub2, but sent by Devices for debugging
typedef enum
{
	SUREFLAP_ZIGBEE_TX_STAT_SUCCESSES,
	SUREFLAP_ZIGBEE_TX_STAT_FAILED_ACK_SENDS,
	SUREFLAP_ZIGBEE_TX_STAT_GOOD_TRANSMISSIONS,
	SUREFLAP_ZIGBEE_TX_STAT_BAD_TRANSMISSIONS,
	SUREFLAP_ZIGBEE_TX_STAT_BACKSTOP

} SureFlapZigbeeTxStateIndexes;
//------------------------------------------------------------------------------

typedef struct
{
	uint64_t mac_address;
	bool found_ping;					// set when a ping has been received
	bool report_ping;					// set when a ping should be reported
	uint32_t transmission_timestamp;
	uint32_t reply_timestamp;
	uint32_t ping_attempts;
	uint32_t num_bad_pings;
	uint32_t num_good_pings;
	uint8_t ping_rss;
	uint8_t ping_res[4];
	uint8_t ping_value;
	uint32_t hub_rssi_sum;
	uint32_t device_rssi_sum;

} SureFlapZigbeePingStatsT;
//------------------------------------------------------------------------------

typedef struct
{
	//including header and parity after payload
	uint16_t PacketLength;
	uint8_t SequenceNumber;

	//should be PACKET_TYPE?
	SureFlapZigbeePacketTypes PacketType;
	uint64_t SourceAddress;
	uint64_t DestinationAddress;
	uint16_t Crc;

	//received signal strength.
	uint8_t RSS;

	//sizeof = 24
	uint8_t Spare;

} SureFlapZigbeePacketHeaderT;
//------------------------------------------------------------------------------

typedef struct
{
	SureFlapZigbeePacketHeaderT Header;
	uint8_t Payload[SUREFLAP_ZIGBEE_MAX_PAYLOAD_SIZE];

} SureFlapZigbeePacketT;
//------------------------------------------------------------------------------

typedef union
{
	//SureFlapZigbeePacketT Packet;
	struct
	{
		SureFlapZigbeePacketHeaderT Header;
		uint8_t Payload[SUREFLAP_ZIGBEE_MAX_PAYLOAD_SIZE];
	};

	uint8_t	Data[sizeof(SureFlapZigbeePacketT)];

} SF_ZigbeeRxPacketT;
//------------------------------------------------------------------------------

typedef struct
{
	uint32_t Timeout;
	SureFlapZigbeeParingRequestSources Source;

	struct
	{
		uint8_t IsEnable : 1;
	};

} SureFlapZigbeeParingRequestT;
//------------------------------------------------------------------------------
// used to transfer received message between SureNet and SureNetDriver
typedef struct
{
	// Source address
    uint64_t uiSrcAddr;

    uint64_t uiDstAddr;
    uint8_t ucBufferLength;
    uint8_t ucRSSI;

    // Receive buffer
    uint8_t ucRxBuffer[127];

} SF_ZigbeeRxBufferT;
//------------------------------------------------------------------------------
//used for the mailbox indicating a successful association
typedef struct
{
    uint64_t DeviceAddress;
    uint8_t DeviceType;
    uint8_t DeviceRSSI;

    // who put us in association mode in the first place
    SureFlapZigbeeParingRequestT Source;

} SureFlapZigbeeAssociationInfoT;
//------------------------------------------------------------------------------
/** This type definition of a structure can store the short address and the extended address of a device. */
typedef struct
{
	uint8_t RSSI;
	uint8_t Type;
	uint16_t ShortAddress;
	uint64_t IEEE_Address;

} SureFlapZigbeeAssociationT;
//------------------------------------------------------------------------------

typedef struct
{
	uint32_t LastTime;

	void* Content;

} SureFlapZigbeeSynchronizationT;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		uint8_t Sequence;
		uint8_t Value;
	};

	uint8_t Data[2];

} SureFlapZigbeeAckPayloadT;
//------------------------------------------------------------------------------

typedef struct
{
	uint8_t SequenceAcknowledged;
	uint8_t AckNack;
	uint16_t TimeOut;

} SF_ZigbeeAcknowledgeElementT;
//------------------------------------------------------------------------------

typedef enum
{
	SF_ZigbeeRequestIdle,
	SF_ZigbeeRequestIsEnable

} SF_ZigbeeRequestStates;
//------------------------------------------------------------------------------

typedef enum
{
	SF_ZigbeeRequestNoError,
	SF_ZigbeeRequestAccept,
	SF_ZigbeeRequestTimeout

} SF_ZigbeeRequestResults;
//------------------------------------------------------------------------------

typedef struct
{
	ObjectBaseT Object;
	
	SureFlapZigbeeAdapterBaseT Adapter;

	SureFlapZigbeeAssociationT Associations[SUREFLAP_NUMBER_OF_DEVICES];

	SureFlapZigbeeAssociationInfoT AssociationInfo;
	SureFlapZigbeeParingRequestT ParingRequest;
	
	void* DestinationDevice;

	struct
	{
		uint32_t IsStarted : 1;
		uint32_t ParingEnable : 1;
		uint32_t EndSession : 1;
		uint32_t RequestConfirmation : 1;
	};

	SF_ZigbeeRequestStates RequestState;
	SF_ZigbeeRequestResults RequestResult;

	SureFlapZigbeeSynchronizationT Synchronization;

	uint16_t PanId;
	uint16_t CurrentChannel;
	uint16_t CurrentChannelPage;

	uint8_t BeaconPayload[SUREFLAP_ZIGBEE_BEACON_PAILOAD_SIZE];
	SureFlapZigbeeAckPayloadT AckPayload;

	int8_t TxPowerPerChannel[SUREFLAP_ZIGBEE_CHANNELS_COUNT];

	SF_ZigbeeRxPacketT RxPacket;
	SF_ZigbeeRxBufferT RxBuffer;

	//SF_ZigbeeAcknowledgeElementT AcknowledgeQueue[SUREFLAP_ZIGBEE_ACKNOWLEDGE_QUEUE_SIZE];
	//SF_ZigbeeAcknowledgeElementT DataAcknowledgeQueue[SUREFLAP_ZIGBEE_DATA_ACKNOWLEDGE_QUEUE_SIZE];

	uint8_t TxSequenceNumber;

	uint32_t RequestTimeStamp;
	uint16_t RequestTimeout;
	  
} SureFlapZigbeeT;
//==============================================================================
//macros:


//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SUREFLAP_ZIGBEE_TYPES_H
#endif //SUREFLAP_COMPONENT_ENABLE
