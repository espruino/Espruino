/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * CC3000 WiFi Interface
 * ----------------------------------------------------------------------------
 */

// from jsutils
void *memcpy(void *dst, const void *src, unsigned int size);
extern int errno;
#define F_GETFL (0)
#define F_SETFL (0)
#define O_NONBLOCK (0)
#define EINPROGRESS (0)
#define EWOULDBLOCK (0)

#define INADDR_ANY      ((unsigned int) 0x00000000)

#ifndef __BOARD_SPI_H
#define __BOARD_SPI_H

#define SPI_VERSION_NUMBER 2

#include <stdint.h>

#define WLAN_SPI          EV_SPI1
#define WLAN_CLK_PIN      (Pin)(JSH_PORTB_OFFSET + 3)
#define WLAN_MISO_PIN     (Pin)(JSH_PORTB_OFFSET + 4)
#define WLAN_MOSI_PIN     (Pin)(JSH_PORTB_OFFSET + 5)
#define WLAN_EN_PIN       (Pin)(JSH_PORTB_OFFSET + 7)
#define WLAN_IRQ_PIN      (Pin)(JSH_PORTB_OFFSET + 8) // active low
#define WLAN_CS_PIN       (Pin)(JSH_PORTB_OFFSET + 6)

#ifndef NULL
#define NULL							(uint32_t)0x00
#endif

void SpiInit(void);
void SpiFlushRxFifo(void);

void SpiReadWriteString(uint32_t ulTrueFalse, const uint8_t *ptrData, uint32_t ulDataSize);
void SpiReadWriteStringInt(uint32_t ulTrueFalse, const uint8_t *ptrData, uint32_t ulDataSize);

void SpiClearInterruptFlag(void);
void SpiInterruptDisable(void);
void SpiInterruptEnable(void);
void SpiRxInterruptClkInit(void);

void SpiDelayOneSecond(void);
void SysCtlDelay(unsigned long ulDelay);
void SpiClose(void);
long SpiWrite(unsigned char *pUserBuffer, unsigned short usLength);
void SpiResumeSpi(void);
long ReadWlanInterruptPin(void);
void WlanInterruptEnable(void);
void WlanInterruptDisable(void);

void SpiIntGPIOHandler(void);

#endif
