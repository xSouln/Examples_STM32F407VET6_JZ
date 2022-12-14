//==============================================================================
#ifndef _SUREFLAP_DEVICE_TYPES_H
#define _SUREFLAP_DEVICE_TYPES_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components_Types.h"
#include "SureFlap_Config.h"
//==============================================================================
//defines:

//==============================================================================
//types:

typedef enum
{
	SUREFLAP_DEVICE_TYPE_UNKNOWN = 0,
	SUREFLAP_DEVICE_TYPE_HUB,			// iHB - Hub
	SUREFLAP_DEVICE_TYPE_REPEATER,		// Never implemented
	SUREFLAP_DEVICE_TYPE_CAT_FLAP,		// iMPD - Pet Door Connect
	SUREFLAP_DEVICE_TYPE_FEEDER,			// iMPF - Microchip Pet Feeder Connect
	SUREFLAP_DEVICE_TYPE_PROGRAMMER,		// Production Programmer
	SUREFLAP_DEVICE_TYPE_DUALSCAN,		// iDSCF - Cat Flap Connect
	SUREFLAP_DEVICE_TYPE_FEEDER_LITE,	// MPF2 - not connect
	SUREFLAP_DEVICE_TYPE_POSEIDON,		// iCWS - Felaqua
	SUREFLAP_DEVICE_TYPE_BACKSTOP		// Should always be last.

} SureFlapDeviceTypes;
//------------------------------------------------------------------------------

typedef enum
{
	SUREFLAP_DEVICE_STATUS_HAS_NO_DATA,
	SUREFLAP_DEVICE_STATUS_HAS_DATA,
	SUREFLAP_DEVICE_STATUS_DETACH,
	SUREFLAP_DEVICE_STATUS_SEND_KEY,
	SUREFLAP_DEVICE_STATUS_DONT_SEND_KEY,
	SUREFLAP_DEVICE_STATUS_RCVD_SEGS,
	SUREFLAP_DEVICE_STATUS_SEGS_COMPLETE,
	
} SureFlapDeviceDataStatus;
//------------------------------------------------------------------------------

typedef enum
{
	SUREFLAP_HUB_OPERATION_INIT,
	SUREFLAP_HUB_OPERATION_CONVERSATION_START,
	SUREFLAP_HUB_OPERATION_CONVERSATION_PET_DOOR_PAUSE,
	SUREFLAP_HUB_OPERATION_CONVERSATION_SEND,
	SUREFLAP_HUB_OPERATION_CONVERSATION_WAIT_FOR_DATA_ACK,
	SUREFLAP_HUB_OPERATION_CONVERSATION_RECEIVE,
	SUREFLAP_HUB_OPERATION_HOP,
	SUREFLAP_HUB_OPERATION_REPEATER,
	SUREFLAP_HUB_OPERATION_WAIT,
	SUREFLAP_HUB_OPERATION_CONVERSATION_WAIT_FOR_DETACH_ACK,
	SUREFLAP_HUB_OPERATION_CONVERSATION_WAIT_FOR_SECURITY_ACK,
	SUREFLAP_HUB_OPERATION_SEND_END_SESSION,
	SUREFLAP_HUB_OPERATION_SEND_PING,
	SUREFLAP_HUB_OPERATION_SEND_PING_WAIT,
	SUREFLAP_HUB_OPERATION_END_OR_RECEIVE,

} SureFlapHubOperations;
//------------------------------------------------------------------------------

typedef enum
{
	SUREFLAP_HUB_HANDLER_IDLE,
	SUREFLAP_HUB_HANDLER_BUSY

} SureFlapHubHandlerState;
//------------------------------------------------------------------------------

typedef enum
{
	SUREFLAP_DEVICE_ENCRYPTION_CRYPT_BLOCK_XTEA,
	SUREFLAP_DEVICE_ENCRYPTION_CHACHA,
	SUREFLAP_DEVICE_ENCRYPTION_AES128,

} SureFlapDeviceEncryptionTypes;
//------------------------------------------------------------------------------

typedef enum
{
	SUREFLAP_DEVICE_ASLEEP,
	SUREFLAP_DEVICE_AWAKE,

} SureFlapDeviceSleepStatus;
//------------------------------------------------------------------------------

typedef struct
{
	SureFlapDeviceSleepStatus Mode;
	SureFlapDeviceDataStatus Data;

} SureFlapDeviceAwakeStatus;
//------------------------------------------------------------------------------

typedef enum
{
	SUREFLAP_DEVICE_SECURITY_KEY_OK,
	SUREFLAP_DEVICE_SECURITY_KEY_RENEW,

} SureFlapDeviceSecureKeyState;
//------------------------------------------------------------------------------

typedef struct
{
	uint8_t Valid : 1;
	uint8_t Online : 1;
	SureFlapDeviceTypes DeviceType : 5;
	uint8_t Associated : 1;

} SureFlapDeviceStateT;
//------------------------------------------------------------------------------

typedef struct
{
	uint64_t MAC_Address;
	SureFlapDeviceStateT State;

	//current status of door locks, sent with every SUREFLAP_DEVICE_AWAKE
	uint8_t LockStatus;

	//signal strength seen from device
	uint8_t DeviceRSSI;

	//signal strength seen from hub
	uint8_t HubRSSI;

	//This is a problem because we store the array of SureFlapDeviceStatusT's in NVM, which is very small.
	//actually because of alignment we use 32 bits anyway
	//now store some other per device useful information in gap
	uint32_t LastHeardFrom;

} SureFlapDeviceStatusT;
//------------------------------------------------------------------------------

typedef struct
{
    uint8_t SecureKeyInvalid;

    //used to track sending of security key to devices
    SureFlapDeviceSecureKeyState SendSecurityKey;
    SureFlapDeviceAwakeStatus AwakeStatus;

    // used to remember the minutes timestamp of last time the device communicated.
    uint8_t DeviceMinutes;

    uint8_t TxEndCount;
    uint8_t SendDetach;

	uint8_t PingValue;
	uint8_t SecurityKeyUses;
	SureFlapDeviceEncryptionTypes EncryptionType;

	struct
	{
		uint8_t DeviceWebIsConnected : 1;
		uint8_t SendPing : 1;
	};

} SureFlapDeviceStatusExtraT;
//------------------------------------------------------------------------------
/** This type definition of a structure can store the short address and the extended address of a device. */
typedef struct
{
	uint8_t RSSI;
	uint8_t Type;
	uint16_t ShortAddress;
	uint64_t IEEE_Address;

} SureFlapDeviceAssociationT;
//------------------------------------------------------------------------------

typedef union
{
	uint8_t InByte[SUREFLAP_DEVICE_SECURITY_KEY_SIZE];
	uint32_t InWord[SUREFLAP_DEVICE_SECURITY_KEY_SIZE / sizeof(uint32_t)];

} SureFlapDeviceSecurityKeyT;
//------------------------------------------------------------------------------

typedef struct
{
	SureFlapDeviceStatusT Status;
	SureFlapDeviceStatusExtraT StatusExtra;
	SureFlapDeviceAssociationT Association;
	SureFlapDeviceSecurityKeyT SecurityKey;

	//sizeof(SecretKey) = CHACHA_MAX_KEY_SZ
	uint8_t SecretKey[32];
	  
} SureFlapDeviceT;
//------------------------------------------------------------------------------

typedef struct
{
	uint32_t LastTime;

} SureFlapDeviceSynchronizationT;
//------------------------------------------------------------------------------

typedef struct
{
	ObjectBaseT Object;

	SureFlapDeviceT Devices[SUREFLAP_NUMBER_OF_DEVICES];

	SureFlapHubOperations HubOperation;
	uint8_t DeviceHandlerIndex;
	uint32_t OperationTimeOut;

	SureFlapHubHandlerState HubHandlerState;
	SureFlapDeviceT* ProcessedDevice;

	SureFlapDeviceSynchronizationT Synchronization;

} SureFlapDeviceControlT;
//==============================================================================
//macros:


//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SUREFLAP_DEVICE_TYPES_H
