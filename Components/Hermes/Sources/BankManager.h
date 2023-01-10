/*****************************************************************************
*
* SUREFLAP CONFIDENTIALITY & COPYRIGHT NOTICE
*
* Copyright � 2013-2021 Sureflap Limited.
* All Rights Reserved.
*
* All information contained herein is, and remains the property of Sureflap 
* Limited.
* The intellectual and technical concepts contained herein are proprietary to
* Sureflap Limited. and may be covered by U.S. / EU and other Patents, patents 
* in process, and are protected by copyright law.
* Dissemination of this information or reproduction of this material is 
* strictly forbidden unless prior written permission is obtained from Sureflap 
* Limited.
*
* Filename: BankManager.h   
* Author:   Tom Monkhouse
* Purpose:     
*             
*             
*
**************************************************************************/

#include <stdint.h>
#include <stdbool.h>

// DCP Config
#define KEY_WORD_SWAP	0x00080000
#define BYTE_WORD_SWAP	0x00040000
#define CIPHER_ENCRYPT	0x0100
#define ENABLE_CIPHER	0x0020
#define DECR_SEMAPHORE	0x0002

#define BM_ENCRYPTION_TIMEOUT	0x10000	// # of loops to iterate before timing out.
#define PRINT_BANK_MANAGER		false

#if PRINT_BANK_MANAGER
#define bm_printf(...)	zprintf(MEDIUM_IMPORTANCE, __VA_ARGS__)
#else
#define bm_printf(...)
#endif

typedef enum
{
	BANK_MARK_ROLLED_BACK	= -2,
	BANK_MARK_UPDATING		= -1,
	BANK_MARK_UNINITIALISED = 0,
	BANK_MARK_DEFAULT		= 1,
	BANK_MARK_MAX = 0x7FFFFFFF
} BANK_MARK;

typedef enum
{
	BM_BANK_UNKONWN,
	BM_BANK_A,
	BM_BANK_B
} BM_BANK;

typedef struct
{
	uint32_t			descriptor_hash;		// Bit-wise XOR of descriptor.
	BANK_MARK			bank_mark;				// Incrementing count for deciding between banks.
	uint32_t			vector_table_offset;	// Offset from bank start for vector table address.
	uint32_t			image_size;				// Size of the image in the bank.
	void*				opposite;				// Points to the address of the opposite bank.
	uint32_t			watchdog_resets;		// Incremented when reset was caused by watchdog.
	bool				encrypted;				// Whether encryption is enabled on this bank.
	BM_BANK				bank;
	uint8_t				garble[2];					// Padding, that also ensures non-zeroness.
} BANK_DESCRIPTOR;

bool	BM_Init(void);	// Required before encryption.
BM_BANK	BM_GetCurrentBank(void);
BM_BANK BM_ReportBank(void);
bool	BM_EnscribeData(uint8_t* data, uint32_t bank_address, uint32_t size);
bool	BM_TranscribeBank(void);
bool	BM_Encrypt(uint8_t* destination, uint8_t* source, uint32_t size);
void	BM_DetermineBankToBoot(void);
void	BM_ResolveBankUse(void);
void	BM_ConfirmBank(bool force_encrypted);
void	BM_BankSwitch(bool bank_b, bool encrypted);	// Sits in STARTUP.
void 	BM_SetBankMark(BANK_MARK new_mark);
