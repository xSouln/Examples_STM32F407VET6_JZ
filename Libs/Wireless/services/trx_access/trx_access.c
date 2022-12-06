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
#include "spi.h"

static irq_handler_t irq_hdl_trx = NULL;

void trx_spi_init(void)
{

}

void PhyReset(void)
{
	/* Ensure control lines have correct levels. */
	TRX_RST_HIGH();
	TRX_SLP_TR_LOW();

	/* Wait typical time of timer TR1. */
	TRX_DELAY(330);

	TRX_RST_LOW();
	TRX_DELAY(10);
	TRX_RST_HIGH();
}

void trx_irq_handler(void)
{
	if (irq_hdl_trx)
	{
		irq_hdl_trx();
	}
}

static void trx_wait_spi()
{
	while (__HAL_SPI_GET_FLAG(&hspi2, SPI_FLAG_BSY))
	{

	}
}

uint8_t trx_reg_read(uint8_t addr)
{
	uint8_t register_value = 0;
	HAL_StatusTypeDef result;

	/*Saving the current interrupt status & disabling the global interrupt
	**/
	ENTER_TRX_CRITICAL_REGION();

	/* Prepare the command byte */
	addr |= READ_ACCESS_COMMAND;

	/* Start SPI transaction by pulling SEL low */
	TRX_SELECT_DEVICE();

	/* Send the Read command byte */
	result = HAL_SPI_Transmit(&hspi2, &addr, sizeof(addr), 100);

	/* Do dummy read for initiating SPI read */
	result = HAL_SPI_Receive(&hspi2, &register_value, sizeof(register_value), 100);

	//result = HAL_SPI_TransmitReceive(&hspi2, packet, packet, sizeof(packet), 100);

	/* Stop the SPI transaction by setting SEL high */
	trx_wait_spi();

	TRX_DESELECT_DEVICE();

	/*Restoring the interrupt status which was stored & enabling the global
	 * interrupt */
	LEAVE_TRX_CRITICAL_REGION();

	return result == HAL_OK ? register_value : 0;
}

void trx_reg_write(uint8_t addr, uint8_t data)
{
	HAL_StatusTypeDef result;

	/*Saving the current interrupt status & disabling the global interrupt
	**/
	ENTER_TRX_CRITICAL_REGION();

	/* Prepare the command byte */
	addr |= WRITE_ACCESS_COMMAND;

	/* Start SPI transaction by pulling SEL low */
	TRX_SELECT_DEVICE();

	/* Send the Read command byte */
	result = HAL_SPI_Transmit(&hspi2, &addr, sizeof(addr), 100);

	result = HAL_SPI_Transmit(&hspi2, &data, sizeof(data), 100);

	/* Stop the SPI transaction by setting SEL high */
	trx_wait_spi();

	TRX_DESELECT_DEVICE();

	LEAVE_TRX_CRITICAL_REGION();

	UNUSED(result);
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
	HAL_StatusTypeDef result;
	uint8_t command = TRX_CMD_FR;
	uint8_t buffer[0xff];

	buffer[0] = TRX_CMD_FR;

	/*Saving the current interrupt status & disabling the global interrupt
	**/
	ENTER_TRX_CRITICAL_REGION();

	/* Start SPI transaction by pulling SEL low */

	TRX_SELECT_DEVICE();

	/* Send the command byte */
	//result = HAL_SPI_Transmit(&hspi2, &command, sizeof(command), 100);

	//result = HAL_SPI_Receive(&hspi2, data, length, 100);

	result = HAL_SPI_TransmitReceive(&hspi2, buffer, buffer, sizeof(command) + length, 100);

	trx_wait_spi();

	memcpy(data, buffer + 1, length);

	/* Stop the SPI transaction by setting SEL high */
	TRX_DESELECT_DEVICE();

	/*Restoring the interrupt status which was stored & enabling the global
	 * interrupt */
	LEAVE_TRX_CRITICAL_REGION();

	UNUSED(result);
}

void trx_frame_write(uint8_t *data, uint8_t length)
{
	HAL_StatusTypeDef result;
	uint8_t command = TRX_CMD_FW;
	uint8_t buffer[0xff];

	buffer[0] = TRX_CMD_FW;

	/*Saving the current interrupt status & disabling the global interrupt
	**/
	ENTER_TRX_CRITICAL_REGION();

	/* Start SPI transaction by pulling SEL low */
	TRX_SELECT_DEVICE();

	memcpy(buffer + 1, data, length);

	/* Send the command byte */
	result = HAL_SPI_Transmit(&hspi2, buffer, sizeof(command) + length, 100);
	//result = HAL_SPI_Transmit(&hspi2, data, length, 100);

	trx_wait_spi();

	/* Stop the SPI transaction by setting SEL high */
	TRX_DESELECT_DEVICE();

	/*Restoring the interrupt status which was stored & enabling the global
	 * interrupt */
	LEAVE_TRX_CRITICAL_REGION();

	UNUSED(result);
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
	HAL_StatusTypeDef result;
	uint8_t command = TRX_CMD_SW;

	/*Saving the current interrupt status & disabling the global interrupt
	**/
	ENTER_TRX_CRITICAL_REGION();

	/* Start SPI transaction by pulling SEL low */
	TRX_SELECT_DEVICE();

	/* Send the command byte */
	result = HAL_SPI_TransmitReceive(&hspi2, &command, &command, sizeof(command), 100);

	/* Send the address from which the write operation should start */
	result = HAL_SPI_TransmitReceive(&hspi2, &addr, &addr, sizeof(addr), 100);

	result = HAL_SPI_Transmit(&hspi2, data, length, 100);

	trx_wait_spi();

	/* Stop the SPI transaction by setting SEL high */
	TRX_DESELECT_DEVICE();

	/*Restoring the interrupt status which was stored & enabling the global
	 * interrupt */
	LEAVE_TRX_CRITICAL_REGION();

	UNUSED(result);
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
	HAL_StatusTypeDef result;
	uint8_t command = TRX_CMD_SR;

	TRX_DELAY(1); /* wap_rf4ce */

	/*Saving the current interrupt status & disabling the global interrupt
	**/
	ENTER_TRX_CRITICAL_REGION();

	/* Start SPI transaction by pulling SEL low */
	TRX_SELECT_DEVICE();

	/* Send the command byte */
	result = HAL_SPI_TransmitReceive(&hspi2, &command, &command, sizeof(command), 100);

	/* Send the command byte */
	result = HAL_SPI_TransmitReceive(&hspi2, &addr, &addr, sizeof(addr), 100);

	/* Send the address from which the read operation should start */
	/* Upload the received byte in the user provided location */
	result = HAL_SPI_Receive(&hspi2, data, length, 100);

	trx_wait_spi();

	/* Stop the SPI transaction by setting SEL high */
	TRX_DESELECT_DEVICE();

	/*Restoring the interrupt status which was stored & enabling the global
	 * interrupt */
	LEAVE_TRX_CRITICAL_REGION();

	UNUSED(result);
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
	HAL_StatusTypeDef result;
	uint8_t *odata;
	uint8_t temp = TRX_CMD_SW;

	TRX_DELAY(1); /* wap_rf4ce */

	ENTER_TRX_CRITICAL_REGION();

	/* Start SPI transaction by pulling SEL low */
	TRX_SELECT_DEVICE();

	/* Send the command byte */
	result = HAL_SPI_Transmit(&hspi2, &temp, sizeof(temp), 100);

	/* write SRAM start address */
	result = HAL_SPI_Transmit(&hspi2, &addr, sizeof(addr), 100);

	/* now transfer data */
	odata = idata;

	result = HAL_SPI_TransmitReceive(&hspi2, idata, odata, length, 100);

	/* Stop the SPI transaction by setting SEL high */
	TRX_DESELECT_DEVICE();

	LEAVE_TRX_CRITICAL_REGION();

	UNUSED(result);
}

void trx_spi_disable(void)
{

}

void trx_spi_enable(void)
{

}
