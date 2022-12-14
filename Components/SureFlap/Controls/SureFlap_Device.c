//==============================================================================
//includes:

#include "SureFlap_Device.h"
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

xResult SureFlapDeviceAssociate(SureFlapDeviceControlT* control,
		uint64_t address,
		uint8_t type,
		uint8_t rssi,
		uint16_t* out_short_address)
{
	for (uint8_t i = 0; i < SUREFLAP_NUMBER_OF_DEVICES; i++)
	{
		if (control->Devices[i].Association.ShortAddress != 0
			&& control->Devices[i].Association.IEEE_Address == address)
		{
			*out_short_address = control->Devices[i].Association.ShortAddress;

			return xResultAccept;
		}
	}

	for (uint8_t i = 0; i < SUREFLAP_NUMBER_OF_DEVICES; i++)
	{
		if (control->Devices[i].Association.ShortAddress == 0)
		{
			control->Devices[i].Association.IEEE_Address = address;
			control->Devices[i].Association.Type = type;
			control->Devices[i].Association.RSSI = rssi;
			control->Devices[i].Association.ShortAddress = i + 0x0001;
			*out_short_address = control->Devices[i].Association.ShortAddress;

			return xResultAccept;
		}
	}

	return xResultError;
}
//------------------------------------------------------------------------------

SureFlapDeviceAssociationT* SureFlapDeviceGetAssociationFrom_MAC(SureFlapDeviceControlT* control, uint64_t mac)
{
	for(uint8_t i = 0; i < SUREFLAP_NUMBER_OF_DEVICES; i++)
	{
		if (control->Devices[i].Association.IEEE_Address == mac)
		{
			return &control->Devices[i].Association;
		}
	}
	return 0;
}
//------------------------------------------------------------------------------
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

xResult SureFlapDeviceAcceptPairing(SureFlapDeviceControlT* control, uint64_t address)
{
	SureFlapDeviceAssociationT* association = SureFlapDeviceGetAssociationFrom_MAC(control, address);

	if (!association)
	{
		return xResultNotFound;
	}

	for (uint8_t i = 0; i < SUREFLAP_NUMBER_OF_DEVICES; i++)
	{
		if (control->Devices[i].Status.MAC_Address == address)
		{
			control->Devices[i].Status.State.Valid = true;
			control->Devices[i].Status.State.Associated = true;
			control->Devices[i].Status.State.DeviceType = association->Type;
			control->Devices[i].Status.DeviceRSSI = association->RSSI;

			return xResultAccept;
		}
	}

	for (uint8_t i = 0; i < SUREFLAP_NUMBER_OF_DEVICES; i++)
	{
		if (!control->Devices[i].Status.State.Valid)
		{
			control->Devices[i].Status.State.Valid = true;
			control->Devices[i].Status.State.Associated = true;
			control->Devices[i].Status.MAC_Address = address;
			control->Devices[i].Status.State.DeviceType = association->Type;
			control->Devices[i].Status.DeviceRSSI = association->RSSI;

			return xResultAccept;
		}
	}

	return xResultError;
}
//------------------------------------------------------------------------------

xResult SureFlapDeviceResetAssociations(SureFlapDeviceControlT* control)
{
	for (uint8_t i = 0; i < SUREFLAP_NUMBER_OF_DEVICES; i++)
	{
		memset(&control->Devices[i].Association, 0, sizeof(SureFlapDeviceAssociationT));
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------

void _SureFlapDeviceHandler(SureFlapDeviceControlT* hub)
{
	//network->Adapter.Interface->Handler(network);
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
			hub->DeviceControl.Devices[i].StatusExtra.AwakeStatus.Mode = SUREFLAP_DEVICE_ASLEEP;
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
