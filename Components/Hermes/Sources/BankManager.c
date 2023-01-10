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
* Filename: Babel.c    
* Author:   Tom Monkhouse
* Purpose:     

**************************************************************************/
 /*
 * Bank Manager (Griphook) arranges the separate firmware banks, their encryption,
 * switching, and confirmation.
 * Tom Monkhouse
 * 09/06/2020
 *
 */

#include "FreeRTOS.h"
#include "hermes.h"
#include "BankManager.h"

extern uint32_t		m_bank_a_start;
extern uint32_t		m_bank_a_end;
extern uint32_t		m_bank_b_start;
extern uint32_t		m_bank_a_size;
extern uint32_t		m_bank_b_size;
extern uint32_t		m_bank_a_descriptor;
extern uint32_t		m_bank_b_descriptor;
const volatile BANK_DESCRIPTOR*	CurrentDescriptor;
const BANK_DESCRIPTOR	BankA_Descriptor = {0, BANK_MARK_DEFAULT, (uint32_t)&m_bank_a_start, (uint32_t)&m_bank_a_size, (void*)&m_bank_b_descriptor, 0, false, BM_BANK_A, {0x73, 0x17}};
const BANK_DESCRIPTOR	BankB_Descriptor = {0, BANK_MARK_UNINITIALISED, (uint32_t)&m_bank_b_start, (uint32_t)&m_bank_b_size, (void*)&m_bank_a_descriptor, 0, true, BM_BANK_B, {0x89, 0xC4}};

BM_BANK BM_GetCurrentBank(void)
{
	if( (CurrentDescriptor != &BankA_Descriptor) && (CurrentDescriptor != &BankB_Descriptor) )
	{
		return BM_BANK_UNKONWN;
	}
	return CurrentDescriptor->bank;
}

BM_BANK BM_ReportBank(void)
{
	BM_BANK	ret = BM_GetCurrentBank();
	zprintf(HIGH_IMPORTANCE, "\r\n\t[~~~~~~~ Bank In Use: ");
	switch( ret )
	{
		case BM_BANK_A:
			zprintf(HIGH_IMPORTANCE, "A");
			break;
			
		case BM_BANK_B:
			zprintf(HIGH_IMPORTANCE, "B");
			break;
			
		default:
			zprintf(HIGH_IMPORTANCE, "Unknown!");
			break;
	}
	
	zprintf(HIGH_IMPORTANCE, " ~~~~~~~]\r\n");
	return ret;
}

bool BM_EnscribeData(uint8_t* data, uint32_t bank_address, uint32_t size)
{
	uint32_t	buffer_offset, crypt_size, offset, i;
	BM_BANK		current_bank = BM_GetCurrentBank();
	
	BM_Init();
	
	switch( current_bank )
	{
		case BM_BANK_A:	// We're in Bank A, so write to Bank B.
			offset = (uint32_t)&m_bank_b_start - (uint32_t)&m_bank_a_start;
			break;
		
		case BM_BANK_B:	// Likewise.
			offset = 0;
			break;
			
		default:
			bm_printf("\r\n\t[@@@@@@@ Unable to resolve current bank! Aborting.\r\n");
			return false;
			break;
	}
	
	while( 0 < size )
	{
		/*
		memset((uint8_t*)bank_page_buffer, 0xFF, FLASH_PAGE_SIZE_BYTES); // FF to avoid overwriting anything already there.
		
		buffer_offset = bank_address % FLASH_PAGE_SIZE_BYTES;
		crypt_size = FLASH_PAGE_SIZE_BYTES - buffer_offset;
		if( crypt_size > size ){ crypt_size = size; }
		
		memcpy((void*)&bank_page_buffer[buffer_offset], (void*)data, crypt_size);
		
		if( false == BM_Encrypt((uint8_t*)&bank_page_buffer[buffer_offset], (uint8_t*)&bank_page_buffer[buffer_offset], crypt_size) )
		{
			bm_printf("]\r\n\t[!!!!!!! Encrpytion Failure @ %p\r\n", bank_address);
			return false;
		}
		
		if( false == hermesFlashRequestWrite((uint8_t*)bank_page_buffer, (uint8_t*)((bank_address + offset) & ~(FLASH_PAGE_SIZE_BYTES - 1)), FLASH_PAGE_SIZE_BYTES, true) )
		{
			bm_printf("]\r\n\t[!!!!!!! Write failure @ %p\r\n", bank_address);
			return false;
		}
		
		for( i=0; i<FLASH_PAGE_SIZE_BYTES; i++ )
		{
			if( *(uint8_t*)(bank_address + offset + i) != bank_page_buffer[i] )
			{
				bm_printf("\r\n\t! Transcribe Error @ %p, 0x%02x vs 0x%02x.", (bank_address + offset + i), bank_page_buffer[i], *(uint8_t*)(bank_address + offset + i));
			}
		}
		
		bank_address += crypt_size;
		size -= crypt_size;
		data += crypt_size;


		if( 0 == (bank_address % FLASH_SECTOR_SIZE_BYTES) )
		{
			bm_printf("+");
		}
		*/
	}
	
	return true;
}

bool BM_TranscribeBank(void)
{
	uint32_t	address = (uint32_t)&m_bank_a_start;
	int32_t		size = 1 + (int32_t)&m_bank_a_end - (int32_t)&m_bank_a_start;
			
	BM_Init();
	
	//volatile uint8_t*	bank_page_buffer = pvPortMalloc(FLASH_PAGE_SIZE_BYTES);
	/*
	if( NULL == bank_page_buffer )
	{
		bm_printf("\r\n\t[!!!!!!! Transcription Aborted - Malloc Failure.\r\n");
		return false;
	}
	*/
			
	bm_printf("\r\n\t[------- Erasing Bank... ");
	
	uint8_t* erase_addr = (uint8_t*)&m_bank_b_start;
	if( BM_BANK_B == BM_GetCurrentBank() )
	{
		erase_addr = (uint8_t*)&m_bank_a_start;
	}
	/*
	if( false == hermesFlashRequestErase(erase_addr, size, true) )
	{
		bm_printf("Failure! Aborting Transcription.\r\n");
		//vPortFree((void*)bank_page_buffer);
		return false;
	}
	*/
	bm_printf("Success!\r\n\t[+++++++ Transcribing Bank!\r\n\t[");
			
	if( false == BM_EnscribeData((uint8_t*)address, address, size) )
	{
		bm_printf("]\r\n\t[xxxxxxx Miserable failure!\r\n");
		return false;
	}
	
	bm_printf("]\r\n\t[------- Great success!\r\n");
	//vPortFree((void*)bank_page_buffer);
	return true;
}

bool BM_Init(void)
{

	return true;
}

bool BM_Encrypt(uint8_t* destination, uint8_t* source, uint32_t size)
{

	return true;
}

// This sets the _other_ bank to be active.
// Note it also zeroes the watchdog reset counter.
void BM_ConfirmBank(bool force_encrypted)
{
	BANK_DESCRIPTOR	new_descriptor;
	memcpy(&new_descriptor, CurrentDescriptor->opposite, sizeof(BANK_DESCRIPTOR));
	
	if( true == force_encrypted ){ new_descriptor.encrypted = true; }
	new_descriptor.bank_mark = (BANK_MARK)((uint32_t)CurrentDescriptor->bank_mark + 1);
	if( 0 >= new_descriptor.bank_mark ){ new_descriptor.bank_mark = (BANK_MARK)1; }
	new_descriptor.watchdog_resets = 0;	// clear watchdog reset counter
	
	new_descriptor.descriptor_hash = 0;
	for( uint32_t i = 4; i < sizeof(BANK_DESCRIPTOR) / sizeof(uint32_t); i++ )
	{
		new_descriptor.descriptor_hash ^= *(((uint32_t*)&new_descriptor) + i);
	}
	
	hermesFlashRequestErase(CurrentDescriptor->opposite, sizeof(BANK_DESCRIPTOR), true);
	hermesFlashRequestWrite((uint8_t*)&new_descriptor, CurrentDescriptor->opposite, sizeof(BANK_DESCRIPTOR), true);
    DCACHE_InvalidateByRange((uint32_t )CurrentDescriptor->opposite, sizeof(BANK_DESCRIPTOR));	
}

/**************************************************************
 * Function Name   : BM_SetBankMark
 * Description     : Sets the bank mark to a value specified as a parameter
 *                 : Intended to set the bank mark to BANK_MARK_UPDATING or BANK_MARK_ROLLED_BACK
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void BM_SetBankMark(BANK_MARK new_mark)
{
	BANK_DESCRIPTOR	new_descriptor;
	memcpy(&new_descriptor, CurrentDescriptor->opposite, sizeof(BANK_DESCRIPTOR));
	
	new_descriptor.bank_mark = new_mark;
	
	new_descriptor.descriptor_hash = 0;
	for( uint32_t i = 4; i < sizeof(BANK_DESCRIPTOR) / sizeof(uint32_t); i++ )
	{
		new_descriptor.descriptor_hash ^= *(((uint32_t*)&new_descriptor) + i);
	}
	
	hermesFlashRequestErase(CurrentDescriptor->opposite, sizeof(BANK_DESCRIPTOR), true);
	hermesFlashRequestWrite((uint8_t*)&new_descriptor, CurrentDescriptor->opposite, sizeof(BANK_DESCRIPTOR), true);
    DCACHE_InvalidateByRange((uint32_t )CurrentDescriptor->opposite, sizeof(BANK_DESCRIPTOR));
}


// Switches banks!
void BM_BankSwitch(bool bank_b, bool encrypted)
{

}

// This function is used just after reset to establish which bank is to be run.
// It then selects the appropriate bank.
void BM_DetermineBankToBoot(void)
{
	uint32_t i;
	uint32_t bank_a_desc_hash = 0;
	uint32_t bank_b_desc_hash = 0;
	bool bank_a_valid, bank_b_valid;
	
	for( i=4; i<sizeof(BANK_DESCRIPTOR)/sizeof(uint32_t); i++ )
	{
		bank_a_desc_hash ^= *(((uint32_t*)&BankA_Descriptor)+i);
		bank_b_desc_hash ^= *(((uint32_t*)&BankB_Descriptor)+i);
	}
	
	bank_a_valid = (bank_a_desc_hash == BankA_Descriptor.descriptor_hash);
	bank_b_valid = (bank_b_desc_hash == BankB_Descriptor.descriptor_hash);
	
	if( BankA_Descriptor.bank_mark <= BANK_MARK_UNINITIALISED ){ bank_a_valid = false; }
	if( BankB_Descriptor.bank_mark <= BANK_MARK_UNINITIALISED ){ bank_b_valid = false; }
	
	if( (true == bank_a_valid) && (true == bank_b_valid) )
	{
		if( BankB_Descriptor.bank_mark < BankA_Descriptor.bank_mark ){ bank_b_valid = false; }
	}
	
	if( true == bank_b_valid )
	{
		BM_BankSwitch(true, true);
	} else
	{
		BM_BankSwitch(false, BankA_Descriptor.encrypted);
	}
}

// Work out from how the BEE is set up which bank we are currently in,
// and set up CurrentDescriptor accordingly.
void BM_ResolveBankUse(void)
{


}
