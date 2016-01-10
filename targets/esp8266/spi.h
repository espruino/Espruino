/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Contains ESP8266 board specific functions.
 * ----------------------------------------------------------------------------
 */

#ifndef SPI_H
#define SPI_H

#include "spi_register.h"
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"

// Define SPI hardware modules
#define SPI 0
#define HSPI 1

void spi_init(uint8 spi_no, uint32 baud_rate);
uint32 spi_transaction(uint8 spi_no, uint32 bits, uint32 dout_data);

#define spi_busy(spi_no) READ_PERI_REG(SPI_CMD(spi_no))&SPI_USR

#endif
