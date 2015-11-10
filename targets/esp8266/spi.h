/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (c) 2015 David Ogilvy (MetalPhreak)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains ESP8266 board specific functions.
 * ----------------------------------------------------------------------------
 */

/**
 * The Github project from which this source file came from can be found
 * at:  https://github.com/MetalPhreak/ESP8266_SPI_Driver
 */

#ifndef SPI_APP_H
#define SPI_APP_H

#include "spi_register.h"
#include "ets_sys.h"
#include "osapi.h"
//#include "uart.h"
#include "os_type.h"

//Define SPI hardware modules
#define SPI 0
#define HSPI 1

#define SPI_CLK_USE_DIV 0
#define SPI_CLK_80MHZ_NODIV 1

#define SPI_BYTE_ORDER_HIGH_TO_LOW 1
#define SPI_BYTE_ORDER_LOW_TO_HIGH 0

#ifndef CPU_CLK_FREQ //Should already be defined in eagle_soc.h
#define CPU_CLK_FREQ 80*1000000
#endif

//Define some default SPI clock settings
#define SPI_CLK_PREDIV 10
#define SPI_CLK_CNTDIV 2
#define SPI_CLK_FREQ CPU_CLK_FREQ/(SPI_CLK_PREDIV*SPI_CLK_CNTDIV) // 80 / 20 = 4 MHz





void spi_init(uint8 spi_no);
void spi_init_gpio(uint8 spi_no, uint8 sysclk_as_spiclk);
void spi_clock(uint8 spi_no, uint16 prediv, uint8 cntdiv);
void spi_tx_byte_order(uint8 spi_no, uint8 byte_order);
void spi_rx_byte_order(uint8 spi_no, uint8 byte_order);
uint32 spi_transaction(uint8 spi_no, uint8 cmd_bits, uint16 cmd_data, uint32 addr_bits, uint32 addr_data, uint32 dout_bits, uint32 dout_data, uint32 din_bits, uint32 dummy_bits);

//Expansion Macros
#define spi_busy(spi_no) READ_PERI_REG(SPI_CMD(spi_no))&SPI_USR

#define spi_txd(spi_no, bits, data) spi_transaction(spi_no, 0, 0, 0, 0, bits, (uint32) data, 0, 0)
#define spi_tx8(spi_no, data)       spi_transaction(spi_no, 0, 0, 0, 0, 8,    (uint32) data, 0, 0)
#define spi_tx16(spi_no, data)      spi_transaction(spi_no, 0, 0, 0, 0, 16,   (uint32) data, 0, 0)
#define spi_tx32(spi_no, data)      spi_transaction(spi_no, 0, 0, 0, 0, 32,   (uint32) data, 0, 0)

#define spi_rxd(spi_no, bits) spi_transaction(spi_no, 0, 0, 0, 0, 0, 0, bits, 0)
#define spi_rx8(spi_no)       spi_transaction(spi_no, 0, 0, 0, 0, 0, 0, 8,    0)
#define spi_rx16(spi_no)      spi_transaction(spi_no, 0, 0, 0, 0, 0, 0, 16,   0)
#define spi_rx32(spi_no)      spi_transaction(spi_no, 0, 0, 0, 0, 0, 0, 32,   0)

#endif

