//==============================================================================
#ifndef _XTEA_H
#define _XTEA_H
//------------------------------------------------------------------------------
#include "Libs_Config.h"
#ifdef LIB_XTEA_ENABLE
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include <stdint.h>
#include <stdbool.h>
//==============================================================================
//defines:

#define XTEA_LOOPS			32
#define XTEA_64_BLOCK_SIZE	4
#define XTEA_KEY_SIZE		16
//==============================================================================
//types:

typedef uint16_t XTEA_64_KeyT[XTEA_KEY_SIZE / 2];
//------------------------------------------------------------------------------

typedef union
{
	uint32_t	Word;
	uint16_t	HalfWords[2];
	uint8_t		Bytes[4];

} XTEA_64_BlockT;
//==============================================================================
//functions:

XTEA_64_BlockT XTEA_64_Encode(XTEA_64_BlockT source, XTEA_64_KeyT key);
void XTEA_64_CTR_Encode(uint8_t* source, uint16_t length, XTEA_64_KeyT key);
XTEA_64_BlockT XTEA_64_GenerateMIC(uint8_t* source, uint32_t length, XTEA_64_KeyT key);
void XTEA_64_Encrypt(uint8_t* source, uint32_t length, uint8_t* key);
bool XTEA_64_Decrypt(uint8_t* source, uint32_t length, uint8_t* key);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_XTEA_H
#endif //LIB_XTEA_ENABLE
