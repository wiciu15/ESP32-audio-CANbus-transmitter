/*
 * spi.h
 *
 *  Created on: 2 lut 2021
 *      Author: Wiktor
 */


#include "driver/spi_master.h"

#define GPIO_MCP2515_CS			19
#define GPIO_MCP2515_SCK		17
#define GPIO_MCP2515_MISO		18
#define GPIO_MCP2515_MOSI		5


spi_device_handle_t spi;

void spi0_init();
void spi0_cs_low();
void spi0_cs_high();
uint8_t spi0_transfer(uint8_t data);
