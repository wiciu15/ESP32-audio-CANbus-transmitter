/** @file

 * @brief Library handling spi communication with MCP2515 module
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 *  Created on: 2 lut 2021
 *      Author: Wiktor Burdecki
 */

#include <string.h>

#include "spi.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"


/**@brief Initialize SPI0 peripheral used to communicate with MCP2515 module
 *
 * @details Function configures output pin for CS (chip-select) line and spi peripheral used in
 * communication with MCP2515 chip
 *
 */
void spi0_init(){
	//MCP2515 non-spi pins

	gpio_set_direction(GPIO_MCP2515_CS, GPIO_MODE_OUTPUT);
	gpio_set_level(GPIO_MCP2515_CS, 1);

	esp_err_t ret;

	spi_bus_config_t buscfg={
			.miso_io_num=GPIO_MCP2515_MISO,
			.mosi_io_num=GPIO_MCP2515_MOSI,
			.sclk_io_num=GPIO_MCP2515_SCK,
			.quadwp_io_num=-1,
			.quadhd_io_num=-1,
			.max_transfer_sz=8
	};
	spi_device_interface_config_t devcfg={
			.clock_speed_hz=1000000,           //Clock out at 1 MHz
			.mode=0,                                //SPI mode 0
			.spics_io_num=-1,               //CS pin handled in software
			.queue_size=7,                          //We want to be able to queue 7 transactions at a time
	};
	//Initialize the SPI bus
	ret=spi_bus_initialize(HSPI_HOST, &buscfg, 0);
	ESP_ERROR_CHECK(ret);
	//Attach the LCD to the SPI bus
	ret=spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
}

/**@brief Chip-select pin control
 *
 * @details Software control od CS pin is needed since one edge on CS pin must account for one instruction,
 * 			and each MCP2515 instruction would send/receive different amount of bytes so can't be hardcoded.
 *
 */
void spi0_cs_low(){
	gpio_set_level(GPIO_MCP2515_CS, 0);
}

void spi0_cs_high(){
	gpio_set_level(GPIO_MCP2515_CS, 1);
}

/**@brief Function used to send/receive byte from SPI peripheral
 *
 * @details This function is called to send/receive byte.
 * 			Doing this stuff one byte at a time and using NRF driver is painfully slow.
 * 			Doesn't matter here since receiving 1 CAN frame from MCP2515's internal buffer is still faster than transmitting one
 * 			CAN frame itself (at 33,3kbaud at least)
 *
 * @param[in]   data   byte to send
 */
uint8_t spi0_transfer(uint8_t data){
	//@TODO:that is really slow solution, for higher CAN baudrates might not work - new CAN frames transmitted over CAN BUS while MCU is still reading old one from buffer via SPI
	//		new,faster implementation of reading RXB0 buffer would be needed, this might still work for configuration/status stuff
	uint8_t spi_rx_buffer=0;

	spi_transaction_t t;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8;                     //Command is 8 bits
	t.tx_buffer=&data;               //The data is the cmd itself
	t.rxlength=0;					//equal to t.length
	t.rx_buffer=&spi_rx_buffer;
	spi_device_polling_transmit(spi, &t);  //Transmit!
	return spi_rx_buffer;
}
