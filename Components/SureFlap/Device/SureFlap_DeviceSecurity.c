//==============================================================================
#include "SureFlap/SureFlap_ComponentConfig.h"
#ifdef SUREFLAP_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "SureFlap_Device.h"
#include "rng.h"
#include "wolfssl/wolfcrypt/sha256.h"
#include "wolfssl/wolfcrypt/chacha.h"
#include "XTEA/XTEA.h"
//==============================================================================
//defines:

// Check with Tom as to why this lot was required / and he has put the #defines in the chacha.h header, i.e. modifying WolfSSL?
#define CHACHA_CONST_WORDS	4
#define CHACHA_CONST_BYTES	(CHACHA_CONST_WORDS * sizeof(word32))
//==============================================================================
//types:

typedef union
{
	uint32_t	Words[CHACHA_CHUNK_WORDS];
	ChaCha		WolfStruct;
	struct
	{
		uint8_t		Constant[CHACHA_CONST_BYTES];
		uint8_t		Key[CHACHA_MAX_KEY_SZ];
		uint32_t	Position;
		uint8_t		IV[CHACHA_IV_WORDS];
	};

} CHACHA_ContextT;
//==============================================================================
//variables:


//==============================================================================
//functions:

/**************************************************************
 * Function Name   : sn_CalculateSecretKey
 * Description     : Generates key for use with ChaCha
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void SureFlapDeviceSetSecretKey(SureFlapDeviceT* device)
{
	static const uint8_t obfuscated_constant[] =
	{
		0x29,0x23,0xbe,0x84,0xe1,0x6c,0xd6,0xae,
		0x52,0x90,0x49,0xf1,0xf1,0xbb,0xe9,0xeb,
		0xb3,0xa6,0xdb,0x3c,0x87,0x0c,0x3e,0x99,
		0x24,0x5e,0x0d,0x1c,0x06,0xb7,0x47,0xde,
		0x7a,0x4a,0xd0,0xe3,0xcd,0x4c,0x91,0xc1,
		0x36,0xf4,0x2c,0x82,0x82,0x97,0xc9,0x84,
		0xd5,0x86,0x9a,0x5f,0xef,0x65,0x52,0xf5,
		0x41,0x2d,0x2a,0x3c,0x74,0xd6,0x20,0xbb
	};

	uint8_t cha_cha_key_constant[CHACHA_MAX_KEY_SZ]; // = "Sing, Goddess, of Achilles' rage";
	wc_Sha256 context;

    for(uint8_t i = 0; i < CHACHA_MAX_KEY_SZ; i++)
    {
    	cha_cha_key_constant[i] = obfuscated_constant[i + 32] ^ obfuscated_constant[i];
    }

	wc_InitSha256(&context);
	wc_Sha256Update(&context, (const byte*)cha_cha_key_constant, CHACHA_MAX_KEY_SZ);

	wc_Sha256Update(&context,
			(const byte*)&device->Status.MAC_Address,
			sizeof(device->Status.MAC_Address));

	wc_Sha256Final(&context, (byte*)device->SecretKey);
}
//------------------------------------------------------------------------------

void SureFlapDeviceSetSecurityKey(SureFlapDeviceT* device)
{
	uint32_t key;
	for (uint8_t i = 0; i < SUREFLAP_DEVICE_SECURITY_KEY_SIZE / sizeof(uint32_t); i++)
	{
		HAL_RNG_GenerateRandomNumber(&hrng, &key);
		device->SecurityKey.InWord[i] = key;
	}
}
//------------------------------------------------------------------------------

void SureFlapDeviceChaChaEncrypt(SureFlapDeviceT* device,
		uint8_t* data,
		uint32_t length,
		uint32_t position)
{
	static CHACHA_ContextT Context;

	wc_Chacha_SetKey(&Context.WolfStruct, device->SecretKey, CHACHA_MAX_KEY_SZ);
	wc_Chacha_SetIV(&Context.WolfStruct, device->SecretKey, position);
	wc_Chacha_Process(&Context.WolfStruct, data, data, length);
}
//------------------------------------------------------------------------------

SureFlapZigbeePacketTypes SureFlapDeviceEncrypt(SureFlapDeviceT* device,
		uint8_t* data,
		uint32_t length,
		uint32_t position)
{
	device->StatusExtra.SecurityKeyUses++;

	switch(device->StatusExtra.EncryptionType)
	{
		case SUREFLAP_DEVICE_ENCRYPTION_CRYPT_BLOCK_XTEA:
		    XTEA_64_Encrypt(data, length, device->SecurityKey.InByte);
			return SUREFLAP_ZIGBEE_PACKET_DATA;

		case SUREFLAP_DEVICE_ENCRYPTION_CHACHA:
			SureFlapDeviceChaChaEncrypt(device, data, length, position);
			return SUREFLAP_ZIGBEE_PACKET_DATA_ALT_ENCRYPTED;

		default:
			return SUREFLAP_ZIGBEE_PACKET_DATA; // Just leave it be.
	}
}

//------------------------------------------------------------------------------
void SureFlapDeviceDecrypt(SureFlapDeviceT* device,
		uint8_t* data,
		uint32_t length,
		uint32_t position)
{
	if(!device)
	{
		return;
	}

	device->StatusExtra.SecurityKeyUses++;

	switch(device->StatusExtra.EncryptionType)
	{
		case SUREFLAP_DEVICE_ENCRYPTION_CRYPT_BLOCK_XTEA:
			// decrypt payload
			XTEA_64_Decrypt(data, length, device->SecurityKey.InByte);
			break;

		case SUREFLAP_DEVICE_ENCRYPTION_CHACHA:
			SureFlapDeviceChaChaEncrypt(device, data, length, position);
			break;

		default:
			// Just leave it be.
			break;
	}
}
//==============================================================================
#endif //SUREFLAP_COMPONENT_ENABLE
