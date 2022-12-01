/*
 * @file trx_access.c
 *
 * @brief Performs interface functionalities between the PHY layer and ASF
 * drivers
 * Copyright (c) 2014-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 *
 */

/*
 * Copyright (c) 2014-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

#include "trx_access.h"
#include "at86rf233.h"
#include "pal.h"

irq_handler_t irq_hdl_trx = NULL;

void trx_spi_init(void)
{
// Nothing to do as it's already done
}

void PhyReset(void)
{
	/* Ensure control lines have correct levels. */
	TRX_RST_HIGH();
	TRX_SLP_TR_LOW();

	/* Wait typical time of timer TR1. */
	HAL_Delay(330);

	TRX_RST_LOW();
	delay_us(10);
	TRX_RST_HIGH();
}

uint8_t trx_reg_read(uint8_t addr)
{
	uint8_t register_value;
    uint8_t masterReceiveBuffer[2] = {0};
    uint8_t masterSendBuffer[2] = {0};

    lpspi_transfer_t masterXfer;
    status_t status;
	/*Saving the current interrupt status & disabling the global interrupt */
	//ENTER_CRITICAL_REGION();    NOT NECESSARY BECAUSE LPSPI_RTOS_Transfer() is mutex'd internally
    /*Setup master transfer*/
    masterXfer.txData = masterSendBuffer;
    masterXfer.rxData = masterReceiveBuffer;
    masterXfer.dataSize = 2;
    masterXfer.configFlags = LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous | kLPSPI_SlaveByteSwap;

    masterSendBuffer[0]=addr | READ_ACCESS_COMMAND; 	/* Prepare the command byte */
    
    status = LPSPI_RTOS_Transfer(&master_rtos_handle, &masterXfer);    

    assert((status==kStatus_Success));
    
    register_value = masterReceiveBuffer[1];

	/*Restoring the interrupt status which was stored & enabling the global interrupt */
	//LEAVE_CRITICAL_REGION();

	return register_value;
}

void trx_reg_write(uint8_t addr, uint8_t data)
{
    uint8_t masterReceiveBuffer[2] = {0};
    uint8_t masterSendBuffer[2] = {0};

    lpspi_transfer_t masterXfer;
    status_t status;

	/*Saving the current interrupt status & disabling the global interrupt */
	//ENTER_CRITICAL_REGION(); NOT NECESSARY BECAUSE LPSPI_RTOS_Transfer() is mutex'd internally

    /*Setup master transfer*/
    masterXfer.txData = masterSendBuffer;
    masterXfer.rxData = masterReceiveBuffer;
    masterXfer.dataSize = 2;
    masterXfer.configFlags = LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous | kLPSPI_SlaveByteSwap;

    masterSendBuffer[0] = addr | WRITE_ACCESS_COMMAND; 	/* Prepare the command byte */
    masterSendBuffer[1] = data; 	/* Prepare the data byte */
    
    status = LPSPI_RTOS_Transfer(&master_rtos_handle, &masterXfer);   

	/*Restoring the interrupt status which was stored & enabling the global interrupt */
	//LEAVE_CRITICAL_REGION();
    assert((status==kStatus_Success));    
}

void trx_irq_init(FUNC_PTR trx_irq_cb)
{
	/*
	 * Set the handler function.
	 * The handler is set before enabling the interrupt to prepare for
	 * spurious
	 * interrupts, that can pop up the moment they are enabled
	 */
	irq_hdl_trx = (irq_handler_t)trx_irq_cb;
}

uint8_t trx_bit_read(uint8_t addr, uint8_t mask, uint8_t pos)
{
	uint8_t ret;
	ret = trx_reg_read(addr);
	ret &= mask;
	ret >>= pos;
	return ret;
}

void trx_bit_write(uint8_t reg_addr, uint8_t mask, uint8_t pos,
		uint8_t new_value)
{
	uint8_t current_reg_value;
	current_reg_value = trx_reg_read(reg_addr);
	current_reg_value &= ~mask;
	new_value <<= pos;
	new_value &= mask;
	new_value |= current_reg_value;
	trx_reg_write(reg_addr, new_value);
}

void trx_frame_read(uint8_t *data, uint8_t length)
{
    uint8_t masterSendBuffer[256] = {0};
    uint8_t masterReceiveBuffer[256] = {0};
//    static volatile uint8_t *dest,*src;
    static volatile uint8_t len;
    lpspi_transfer_t masterXfer;
    status_t status;

    if (length==0)
    { // Suspect this causes an exception in the memcpy call at the end.
        zprintf(HIGH_IMPORTANCE,"trx_frame_read() called with length=0\r\n");
    }
    
	/*Saving the current interrupt status & disabling the global interrupt */
	//ENTER_CRITICAL_REGION(); NOT NECESSARY BECAUSE LPSPI_RTOS_Transfer() is mutex'd internally
    
    /*Setup master transfer*/
    masterXfer.txData = masterSendBuffer;
    masterXfer.rxData = masterReceiveBuffer;
    masterXfer.dataSize = length+1;
    masterXfer.configFlags = LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous | kLPSPI_SlaveByteSwap;

    masterSendBuffer[0] = TRX_CMD_FR; 	/* Prepare the command byte */
    
    status = LPSPI_RTOS_Transfer(&master_rtos_handle, &masterXfer);   

    assert((status==kStatus_Success));
    
//    dest = data;
//    src = &masterReceiveBuffer[1];
    len = length;
    if ((data<(uint8_t *)0x10000) ||
		((data>(uint8_t *)0x20000000) && (data<(uint8_t *)0x20010000)) ||
		((data>(uint8_t *)0x20200000) && (data<(uint8_t *)0x20220000)))
	    memcpy(data,&masterReceiveBuffer[1],length);
    else
	{
		zprintf(HIGH_IMPORTANCE,"BAD DATA POINTER\r\n");
        DbgConsole_Flush();
		while(1);
	}
	/*Restoring the interrupt status which was stored & enabling the global interrupt */
	//LEAVE_CRITICAL_REGION();
}

void trx_frame_write(uint8_t *data, uint8_t length)
{
    uint8_t masterSendBuffer[256] = {0};
    uint8_t masterReceiveBuffer[256] = {0};
    
    lpspi_transfer_t masterXfer;
    status_t status;

	/*Saving the current interrupt status & disabling the global interrupt */
	//ENTER_CRITICAL_REGION(); NOT NECESSARY BECAUSE LPSPI_RTOS_Transfer() is mutex'd internally

    /*Setup master transfer*/
    masterXfer.txData = masterSendBuffer;
    masterXfer.rxData = masterReceiveBuffer;
    masterXfer.dataSize = length+1;
    masterXfer.configFlags = LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous | kLPSPI_SlaveByteSwap;

    masterSendBuffer[0] = TRX_CMD_FW; 	/* Prepare the command byte */
    
    memcpy(&masterSendBuffer[1],data,length);   // copy data into our buffer at offset 1
    
    status = LPSPI_RTOS_Transfer(&master_rtos_handle, &masterXfer);   

    assert((status==kStatus_Success));
    
	/*Restoring the interrupt status which was stored & enabling the global interrupt */
	//LEAVE_CRITICAL_REGION();
}

/**
 * @brief Writes data into SRAM of the transceiver
 *
 * This function writes data into the SRAM of the transceiver
 *
 * @param addr Start address in the SRAM for the write operation
 * @param data Pointer to the data to be written into SRAM
 * @param length Number of bytes to be written into SRAM
 */
void trx_sram_write(uint8_t addr, uint8_t *data, uint8_t length)
{
    uint8_t masterSendBuffer[257] = {0};
    uint8_t masterReceiveBuffer[257] = {0};
    
    lpspi_transfer_t masterXfer;
    status_t status;

	/*Saving the current interrupt status & disabling the global interrupt */
	//ENTER_CRITICAL_REGION(); NOT NECESSARY BECAUSE LPSPI_RTOS_Transfer() is mutex'd internally

    /*Setup master transfer*/
    masterXfer.txData = masterSendBuffer;
    masterXfer.rxData = masterReceiveBuffer;
    masterXfer.dataSize = length+2;
    masterXfer.configFlags = LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous | kLPSPI_SlaveByteSwap;

    masterSendBuffer[0] = TRX_CMD_SW; 	/* Prepare the command byte */
    masterSendBuffer[1] = addr; 	/* Prepare the address byte */
    
    memcpy(&masterSendBuffer[2],data,length);   // copy data into our buffer at offset 2
    
    status = LPSPI_RTOS_Transfer(&master_rtos_handle, &masterXfer);   

    assert((status==kStatus_Success));
    
	/*Restoring the interrupt status which was stored & enabling the global interrupt */
	//LEAVE_CRITICAL_REGION();
}

/**
 * @brief Reads data from SRAM of the transceiver
 *
 * This function reads from the SRAM of the transceiver
 *
 * @param[in] addr Start address in SRAM for read operation
 * @param[out] data Pointer to the location where data stored
 * @param[in] length Number of bytes to be read from SRAM
 */
void trx_sram_read(uint8_t addr, uint8_t *data, uint8_t length)
{
	delay_us(1); /* wap_rf4ce */

    uint8_t masterSendBuffer[257] = {0};
    uint8_t masterReceiveBuffer[257] = {0};

    lpspi_transfer_t masterXfer;
    status_t status;

	/*Saving the current interrupt status & disabling the global interrupt */
	//ENTER_CRITICAL_REGION(); NOT NECESSARY BECAUSE LPSPI_RTOS_Transfer() is mutex'd internally
    
    /*Setup master transfer*/
    masterXfer.txData = masterSendBuffer;
    masterXfer.rxData = masterReceiveBuffer;
    masterXfer.dataSize = length+2;
    masterXfer.configFlags = LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous | kLPSPI_SlaveByteSwap;

    masterSendBuffer[0] = TRX_CMD_SR; 	/* Prepare the command byte */
    masterSendBuffer[1] = addr; 	    /* Prepare the address byte */
    
    status = LPSPI_RTOS_Transfer(&master_rtos_handle, &masterXfer);   

	if (status!=kStatus_Success)
	{
		zprintf(HIGH_IMPORTANCE,"SPI fail, error = %02X\r\n",status);
	    assert((status==kStatus_Success));
	}
	
    memcpy(data,&masterReceiveBuffer[2],length);    
    
	/*Saving the current interrupt status & disabling the global interrupt
	**/
  	//LEAVE_CRITICAL_REGION();
}

/**
 * @brief Writes and reads data into/from SRAM of the transceiver
 *
 * This function writes data into the SRAM of the transceiver and
 * simultaneously reads the bytes.
 *
 * @param addr Start address in the SRAM for the write operation
 * @param idata Pointer to the data written/read into/from SRAM
 * @param length Number of bytes written/read into/from SRAM
 */
void trx_aes_wrrd(uint8_t addr, uint8_t *idata, uint8_t length)
{
    uint8_t masterSendBuffer[257] = {0};
    uint8_t masterReceiveBuffer[257] = {0};
    
    lpspi_transfer_t masterXfer;
    status_t status;

	/*Saving the current interrupt status & disabling the global interrupt */
	//ENTER_CRITICAL_REGION(); NOT NECESSARY BECAUSE LPSPI_RTOS_Transfer() is mutex'd internally

    /*Setup master transfer*/
    masterXfer.txData = masterSendBuffer;
    masterXfer.rxData = masterReceiveBuffer;
    masterXfer.dataSize = length+2;
    masterXfer.configFlags = LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous | kLPSPI_SlaveByteSwap;

    masterSendBuffer[0] = TRX_CMD_SW; 	/* Prepare the command byte */
    masterSendBuffer[1] = addr; 	/* Prepare the address byte */
    
    memcpy(&masterSendBuffer[2],idata,length);   // copy data into our buffer at offset 2
    
    status = LPSPI_RTOS_Transfer(&master_rtos_handle, &masterXfer);   

    memcpy(idata,&masterReceiveBuffer[2],length);   // copy the returned data over the input data
    assert((status==kStatus_Success));
    
	/*Restoring the interrupt status which was stored & enabling the global interrupt */
	//LEAVE_CRITICAL_REGION();
}

void trx_spi_disable(void)
{
// should never be called!
}

void trx_spi_enable(void)
{
// should never be called!
}
