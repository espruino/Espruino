// USED FOR SPI-BASED SD CARDS
/*-----------------------------------------------------------------------*/
/* MMC/SDSC/SDHC (in SPI mode) control module for STM32 Version 1.1.0    */
/* (C) Martin Thomas, 2009 - based on the AVR MMC module (C)ChaN, 2007   */
/*-----------------------------------------------------------------------*/

/* Copyright (c) 2009, Martin Thomas, ChaN
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */


#include "platform_config.h"
#include "jsutils.h"
#include "jshardware.h"
#include "jsparse.h"
#include "jsspi.h"
#include "diskio.h"

/* Card type flags (CardType) */
#define CT_MMC                         0x01
#define CT_SD1                         0x02
#define CT_SD2                         0x04
#define CT_SDC                         (CT_SD1|CT_SD2)
#define CT_BLOCK                       0x08


/* Definitions for MMC/SDC command */
#define CMD0	(0x40+0)	/* GO_IDLE_STATE */
#define CMD1	(0x40+1)	/* SEND_OP_COND (MMC) */
#define ACMD41	(0xC0+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(0x40+8)	/* SEND_IF_COND */
#define CMD9	(0x40+9)	/* SEND_CSD */
#define CMD10	(0x40+10)	/* SEND_CID */
#define CMD12	(0x40+12)	/* STOP_TRANSMISSION */
#define ACMD13	(0xC0+13)	/* SD_STATUS (SDC) */
#define CMD16	(0x40+16)	/* SET_BLOCKLEN */
#define CMD17	(0x40+17)	/* READ_SINGLE_BLOCK */
#define CMD18	(0x40+18)	/* READ_MULTIPLE_BLOCK */
#define CMD23	(0x40+23)	/* SET_BLOCK_COUNT (MMC) */
#define ACMD23	(0xC0+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(0x40+24)	/* WRITE_BLOCK */
#define CMD25	(0x40+25)	/* WRITE_MULTIPLE_BLOCK */
#define CMD55	(0x40+55)	/* APP_CMD */
#define CMD58	(0x40+58)	/* READ_OCR */

/* Card-Select Controls  (Platform dependent) */
#define SELECT()        jshPinOutput(sdCSPin, 0)    /* MMC CS = L */
#define DESELECT()      jshPinOutput(sdCSPin, 1)      /* MMC CS = H */

/*--------------------------------------------------------------------------

   Module Private Functions and Variables

---------------------------------------------------------------------------*/

static volatile DSTATUS Stat = STA_NOINIT;	/* Disk status */
static BYTE CardType;			/* Card type flags */

static Pin sdCSPin = PIN_UNDEFINED; // SD_CS_PIN
static JsVar *sdSPI = 0;

static inline bool chk_power() { return 1; }


/*-----------------------------------------------------------------------*/
/* Setup                                                                 */
/*-----------------------------------------------------------------------*/

bool isSdSPISetup() {
  return sdSPI && jshIsPinValid(sdCSPin);
}


void sdSPISetup(JsVar *spi, Pin csPin) {
  jsvUnLock(sdSPI);
  sdSPI = jsvLockAgainSafe(spi);
  sdCSPin = csPin;
  if (isSdSPISetup()) DESELECT();
}


/* Transmit bytes from MMC via SPI - leave array alone */
static void write_spi_buf (BYTE *data, UINT len)
{
  jsspiSend(sdSPI, JSSPI_NO_RECEIVE, (char*)data, len);
}

/* Transmit bytes from MMC via SPI - writes result back into array */
static void xfer_spi_buf (BYTE *data, UINT len)
{
  jsspiSend(sdSPI, 0, (char*)data, len);
}


/* Receive a byte from MMC via SPI                                       */
static BYTE read_spi_byte (void)
{
  char data = 0xFF;
  jsspiSend(sdSPI, 0, &data, 1);
  return (BYTE)data;
}



/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
BYTE wait_ready (void)
{
	BYTE res;


	JsSysTime endTime = jshGetSystemTime()+jshGetTimeFromMilliseconds(500);	/* Wait for ready in timeout of 500ms */

	read_spi_byte();
	do
		res = read_spi_byte();
	while ((res != 0xFF) && (jshGetSystemTime() < endTime));

	return res;
}



/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void release_spi (void)
{
	DESELECT();
	read_spi_byte();
}

/*-----------------------------------------------------------------------*/
/* Power Control  (Platform dependent)                                   */
/*-----------------------------------------------------------------------*/
/* When the target system does not support socket power control, there   */
/* is nothing to do in these functions and chk_power always returns 1.   */

static
void power_on (void)
{
	/* Deselect the Card: Chip Select high */
	DESELECT();
}

static
void power_off (void)
{
	if (!(Stat & STA_NOINIT)) {
		SELECT();
		wait_ready();
		release_spi();
	}

	// TODO: we should have a way of disabling SPI in the hardware API...

	/* All SPI-Pins to input */
/*	jshPinInput(SD_DO_PIN);
	jshPinInput(SD_DI_PIN);
	jshPinInput(SD_CLK_PIN);*/
	
	Stat |= STA_NOINIT;		/* Set STA_NOINIT */
}


/*-----------------------------------------------------------------------*/
/* Receive a data packet from MMC                                        */
/*-----------------------------------------------------------------------*/

static
bool rcvr_datablock (
	BYTE *buff,			/* Data buffer to store received data */
	UINT btr			/* Byte count (must be multiple of 4) */
)
{
	BYTE token;

	JsSysTime endTime = jshGetSystemTime()+jshGetTimeFromMilliseconds(100); /* Wait for data packet in timeout of 100ms */
	do {
		token = read_spi_byte();
	} while ((token == 0xFF) && (jshGetSystemTime() < endTime));
	if(token != 0xFE) return FALSE;	/* If not valid data token, return with error */

	UINT i;
	for (i=0;i<btr;i++)
	  buff[i] = 0xFF; // make sure we send 0xFF
	xfer_spi_buf(buff, btr);

    BYTE crc[2] = {0xFF,0xFF};
    write_spi_buf(crc, 2); // discard 2 byte CRC

	return TRUE;					/* Return with success */
}



/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
static
bool xmit_datablock (
	const BYTE *buff,	/* 512 byte data block to be transmitted */
	BYTE token			/* Data/Stop token */
)
{
	if (wait_ready() != 0xFF) return FALSE;

	write_spi_buf(&token, 1);	/* Xmit data token */
	if (token != 0xFD) {	/* Is data token */
	    write_spi_buf((BYTE *)buff, 512); /* Xmit the 512 byte data block to MMC */

		BYTE resp[3] = {0xFF,0xFF,0xFF};
		xfer_spi_buf(resp, 3); // discard 2 byte CRC and get 1 byte response
		if ((resp[2] & 0x1F) != 0x05)		/* If not accepted, return with error */
			return FALSE;
	}

	return TRUE;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static
BYTE send_cmd (
	BYTE cmd,		/* Command byte */
	DWORD arg		/* Argument */
)
{
	BYTE n, res;


	if (cmd & 0x80) {	/* ACMD<n> is the command sequence of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) return res;
	}

	/* Select the card and wait for ready */
	DESELECT();
	SELECT();
	if (wait_ready() != 0xFF) {
		return 0xFF;
	}

    n = 0x01;                           /* Dummy CRC + Stop */
    if (cmd == CMD0) n = 0x95;          /* Valid CRC for CMD0(0) */
    if (cmd == CMD8) n = 0x87;          /* Valid CRC for CMD8(0x1AA) */

	/* Send command packet */
    /* Start + Command index */
    /* Argument[31..24] */
    /* Argument[23..16] */
    /* Argument[15..8] */
    /* Argument[7..0] */
    /* CRC */
    BYTE packet[6] = {cmd, (BYTE)(arg >> 24), (BYTE)(arg >> 16), (BYTE)(arg >> 8), (BYTE)(arg), n};
    xfer_spi_buf(packet, 6); // discard 2 byte CRC and get 1 byte response

	/* Receive command response */
	if (cmd == CMD12) read_spi_byte();		/* Skip a stuff byte when stop reading */
	
	n = 10;								/* Wait for a valid response in timeout of 10 attempts */
	do
		res = read_spi_byte();
	while ((res & 0x80) && --n);
	
	return res;			/* Return with the response value */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE drv		/* Physical drive number (0) */
)
{
	BYTE n, cmd, ty, ocr[4];


	if (drv) return STA_NOINIT;			/* Supports only single drive */
	if (Stat & STA_NODISK) return Stat;	/* No card in the socket */

	// try and initialise the SD card - give it a few tries
	int tries = 2;
	do {
	  power_on();             /* Force socket power on */
    for (n = 10; n; n--) read_spi_byte();	/* 80 dummy clocks */
    n = send_cmd(CMD0, 0);
	} while ((n != 1) && (tries-- > 0) && !jspIsInterrupted());

	ty = 0;
	if (n == 1) {			/* Enter Idle state */
	    JsSysTime endTime = jshGetSystemTime()+jshGetTimeFromMilliseconds(1000); /* Initialization timeout of 1000 msec */
		if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDHC */
			for (n = 0; n < 4; n++) ocr[n] = read_spi_byte();		/* Get trailing return value of R7 resp */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {			/* The card can work at vdd range of 2.7-3.6V */
				while ((jshGetSystemTime()<endTime) && send_cmd(ACMD41, 1UL << 30));	/* Wait for leaving idle state (ACMD41 with HCS bit) */
				if ((jshGetSystemTime()<endTime) && send_cmd(CMD58, 0) == 0) {		/* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) ocr[n] = read_spi_byte();
					ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
				}
			}
		} else {							/* SDSC or MMC */
			if (send_cmd(ACMD41, 0) <= 1) 	{
				ty = CT_SD1; cmd = ACMD41;	/* SDSC */
			} else {
				ty = CT_MMC; cmd = CMD1;	/* MMC */
			}
			while ((jshGetSystemTime()<endTime) && send_cmd(cmd, 0));			/* Wait for leaving idle state */
			if (!(jshGetSystemTime()<endTime) || send_cmd(CMD16, 512) != 0)	/* Set R/W block length to 512 */
				ty = 0;
		}
	}
	CardType = ty;
	release_spi();

	if (ty) {			/* Initialization succeeded */
		Stat &= ~STA_NOINIT;		/* Clear STA_NOINIT */
		// TODO: can now increase speed
	} else {			/* Initialization failed */
		power_off();
	}

	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv		/* Physical drive number (0) */
)
{
	if (drv) return STA_NOINIT;		/* Supports only single drive */
	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE drv,			/* Physical drive number (0) */
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Sector count */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {	/* Single block read */
		if (send_cmd(CMD17, sector) == 0)	{ /* READ_SINGLE_BLOCK */
			if (rcvr_datablock(buff, 512)) {
				count = 0;
			}
		}
	}
	else {				/* Multiple block read */
		if (send_cmd(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK */
			do {
				if (!rcvr_datablock(buff, 512)) {
					break;
				}
				buff += 512;
			} while (--count);
			send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
		}
	}
	release_spi();
	
	return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive number (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Sector count */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (Stat & STA_PROTECT) return RES_WRPRT;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {	/* Single block write */
		if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
			&& xmit_datablock(buff, 0xFE))
			count = 0;
	}
	else {				/* Multiple block write */
		if (CardType & CT_SDC) send_cmd(ACMD23, count);
		if (send_cmd(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
			do {
				if (!xmit_datablock(buff, 0xFC)) break;
				buff += 512;
			} while (--count);
			if (!xmit_datablock(0, 0xFD))	/* STOP_TRAN token */
				count = 1;
		}
	}
	release_spi();

	return count ? RES_ERROR : RES_OK;
}
#endif /* _READONLY == 0 */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL != 0
DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive number (0) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	BYTE n, csd[16], *ptr = buff;
	WORD csize;


	if (drv) return RES_PARERR;

	res = RES_ERROR;

	if (ctrl == CTRL_POWER) {
		switch (*ptr) {
		case 0:		/* Sub control code == 0 (POWER_OFF) */
			if (chk_power())
				power_off();		/* Power off */
			res = RES_OK;
			break;
		case 1:		/* Sub control code == 1 (POWER_ON) */
			power_on();				/* Power on */
			res = RES_OK;
			break;
		case 2:		/* Sub control code == 2 (POWER_GET) */
			*(ptr+1) = (BYTE)chk_power();
			res = RES_OK;
			break;
		default :
			res = RES_PARERR;
			break;
		}
	}
	else {
		if (Stat & STA_NOINIT) return RES_NOTRDY;

		switch (ctrl) {
		case CTRL_SYNC :		/* Make sure that no pending write process */
			SELECT();
			if (wait_ready() == 0xFF)
				res = RES_OK;
			break;

		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
				if ((csd[0] >> 6) == 1) {	/* SDC ver 2.00 */
					csize = csd[9] + ((WORD)csd[8] << 8) + 1;
					*(DWORD*)buff = (DWORD)csize << 10;
				} else {					/* SDC ver 1.XX or MMC*/
					n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
					csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
					*(DWORD*)buff = (DWORD)csize << (n - 9);
				}
				res = RES_OK;
			}
			break;

		case GET_SECTOR_SIZE :	/* Get R/W sector size (WORD) */
			*(WORD*)buff = 512;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
			if (CardType & CT_SD2) {	/* SDC ver 2.00 */
				if (send_cmd(ACMD13, 0) == 0) {	/* Read SD status */
					read_spi_byte();
					if (rcvr_datablock(csd, 16)) {				/* Read partial block */
						for (n = 64 - 16; n; n--) read_spi_byte();	/* Purge trailing data */
						*(DWORD*)buff = 16UL << (csd[10] >> 4);
						res = RES_OK;
					}
				}
			} else {					/* SDC ver 1.XX or MMC */
				if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {	/* Read CSD */
					if (CardType & CT_SD1) {	/* SDC ver 1.XX */
						*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
					} else {					/* MMC */
						*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
					}
					res = RES_OK;
				}
			}
			break;

		case MMC_GET_TYPE :		/* Get card type flags (1 byte) */
			*ptr = CardType;
			res = RES_OK;
			break;

		case MMC_GET_CSD :		/* Receive CSD as a data block (16 bytes) */
			if (send_cmd(CMD9, 0) == 0		/* READ_CSD */
				&& rcvr_datablock(ptr, 16))
				res = RES_OK;
			break;

		case MMC_GET_CID :		/* Receive CID as a data block (16 bytes) */
			if (send_cmd(CMD10, 0) == 0		/* READ_CID */
				&& rcvr_datablock(ptr, 16))
				res = RES_OK;
			break;

		case MMC_GET_OCR :		/* Receive OCR as an R3 resp (4 bytes) */
			if (send_cmd(CMD58, 0) == 0) {	/* READ_OCR */
				for (n = 4; n; n--) *ptr++ = read_spi_byte();
				res = RES_OK;
			}
			break;

		case MMC_GET_SDSTAT :	/* Receive SD status as a data block (64 bytes) */
			if (send_cmd(ACMD13, 0) == 0) {	/* SD_STATUS */
				read_spi_byte();
				if (rcvr_datablock(ptr, 64))
					res = RES_OK;
			}
			break;

		default:
			res = RES_PARERR;
			break;
		}

		release_spi();
	}

	return res;
}
#endif /* _USE_IOCTL != 0 */
