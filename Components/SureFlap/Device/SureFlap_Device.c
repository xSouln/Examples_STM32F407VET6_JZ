//==============================================================================
#include "SureFlap/SureFlap_ComponentConfig.h"
#ifdef SUREFLAP_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "SureFlap.h"
#include "rng.h"
//==============================================================================
//defines:

#define SECURITY_KEY_00				0x45  // E
#define SECURITY_KEY_01				0x75  // u
#define SECURITY_KEY_02				0x6c  // l
#define SECURITY_KEY_03				0x65  // e
#define SECURITY_KEY_04				0x72  // r
#define SECURITY_KEY_05				0x60  // '
#define SECURITY_KEY_06				0x73  // s
#define SECURITY_KEY_07				0x20  //
#define SECURITY_KEY_08				0x49  // I
#define SECURITY_KEY_09				0x64  // d
#define SECURITY_KEY_10				0x65  // e
#define SECURITY_KEY_11				0x6e  // n
#define SECURITY_KEY_12				0x74  // t
#define SECURITY_KEY_13				0x69  // i
#define SECURITY_KEY_14				0x74  // t
#define SECURITY_KEY_15				0x79  // y
//==============================================================================
//variables:
#if !SUREFLAP_DEVICE_SECURITY_USE_RANDOM_KEY
static const int8_t secure_key[SUREFLAP_DEVICE_SECURITY_KEY_SIZE] =
{
	SECURITY_KEY_00, SECURITY_KEY_01, SECURITY_KEY_02, SECURITY_KEY_03,
	SECURITY_KEY_04, SECURITY_KEY_05, SECURITY_KEY_06, SECURITY_KEY_07,
	SECURITY_KEY_08, SECURITY_KEY_09, SECURITY_KEY_10, SECURITY_KEY_11,
	SECURITY_KEY_12, SECURITY_KEY_13, SECURITY_KEY_14, SECURITY_KEY_15
};
#endif
//==============================================================================
//functions:

int8_t SureFlapDeviceGetIndexFrom_MAC(SureFlapDeviceControlT* control, uint64_t mac)
{
	for (uint8_t i = 0; i < SUREFLAP_NUMBER_OF_DEVICES; i++)
	{
		if (control->Devices[i].Status.MAC_Address == mac)
		{
			return i;
		}
	}
	return -1;
}
//------------------------------------------------------------------------------
SureFlapDeviceT* SureFlapDeviceGetFrom_MAC(SureFlapDeviceControlT* control, uint64_t mac)
{
	for (uint8_t i = 0; i < SUREFLAP_NUMBER_OF_DEVICES; i++)
	{
		if (control->Devices[i].Status.MAC_Address == mac)
		{
			return &control->Devices[i];
		}
	}
	return NULL;
}
//------------------------------------------------------------------------------

void _SureFlapDeviceHandler(SureFlapDeviceControlT* control)
{
	//network->Adapter.Interface->Handler(network);
	uint32_t time = SureFlapGetTime(control->Object.Parent);

	for (uint8_t i = 0; i < SUREFLAP_NUMBER_OF_DEVICES; i++)
	{
		if(control->Devices[i].Status.Online)
		{
			if((time - control->Devices[i].Status.PacketReceptionTimeStamp) > 5000)
			{
				control->Devices[i].Status.Online = false;
			}
		}

		if(control->Devices[i].HubOperation != SUREFLAP_HUB_OPERATION_CONVERSATION_START)
		{
			if(!control->Devices[i].Status.Online
			|| (time - control->Devices[i].HubOperationTimeStamp) > control->Devices[i].HubOperationTimeout)
			{
				control->Devices[i].HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_START;
			}
		}
	}
}
//------------------------------------------------------------------------------

void _SureFlapDeviceTimeSynchronization(SureFlapDeviceControlT* hub)
{
	//network->Adapter.Interface->TimeSynchronization(hub);
}
//==============================================================================
//initialization:

xResult SureFlapDeviceInit(SureFlapT* hub)
{
	if (hub)
	{
		hub->DeviceControl.Object.Description = nameof(SureFlapDeviceControlT);
		hub->DeviceControl.Object.Parent = hub;

		for (uint8_t i = 0; i < SUREFLAP_NUMBER_OF_DEVICES; i++)
		{
			hub->DeviceControl.Devices[i].StatusExtra.SendDetach = 0;
			hub->DeviceControl.Devices[i].StatusExtra.AwakeStatus.State = SUREFLAP_DEVICE_ASLEEP;
			hub->DeviceControl.Devices[i].StatusExtra.AwakeStatus.Data = SUREFLAP_DEVICE_STATUS_HAS_NO_DATA;
			hub->DeviceControl.Devices[i].StatusExtra.DeviceWebIsConnected = false;
			hub->DeviceControl.Devices[i].StatusExtra.SendSecurityKey = SUREFLAP_DEVICE_SECURITY_KEY_OK;
			hub->DeviceControl.Devices[i].StatusExtra.SecurityKeyUses = 0;
			hub->DeviceControl.Devices[i].StatusExtra.EncryptionType = SUREFLAP_DEVICE_ENCRYPTION_CRYPT_BLOCK_XTEA;

#if SUREFLAP_DEVICE_SECURITY_USE_RANDOM_KEY
			SureFlapDeviceSetSecurityKey(&hub->DeviceControl.Devices[i]);
#else
			memcpy(hub->DeviceControl.Devices[i].SecurityKey, secure_key, sizeof(secure_key));
#endif
		}

		return xResultAccept;
	}
	
	return xResultError;
}
//==============================================================================
#endif //SUREFLAP_COMPONENT_ENABLE
