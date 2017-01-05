/*
 * This file is designed to support spi functions in Espruino for ESP32,
 * a JavaScript interpreter for Microcontrollers designed by Gordon Williams
 *
 * Copyright (C) 2016 by Rhys Williams (wilberforce)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains ESP32 board specific functions.
 * ----------------------------------------------------------------------------
 */

#include "jspininfo.h"
#include "jshardware.h"
#include "driver/gpio.h"
// Using arduino library
#include "esp32-hal-spi.h"

#define UNUSED(x) (void)(x)

// Convert an Espruino pin to an ESP32 pin number.
gpio_num_t pinToESP32Pin(Pin pin);

static spi_t * _spi[VSPI];
static uint32_t  g_lastSPIRead = (uint32_t)-1;

/*
https://hackadaycom.files.wordpress.com/2016/10/esp32_pinmap.png
HSPI  2 //SPI bus normally mapped to pins 12 - 15, but can be matrixed to any pins
15  HSPI SS
14  HSPI SCK
12  HSPI MISO
13  HSPI MOSI
VSPI  3 //SPI bus normally attached to pin:
5   VSPI SS
18  VSPI SCK
19  VSPI MISO
23  VSPI MOSI
*/
//===== SPI =====
int getSPIFromDevice( IOEventFlags device	) {
  switch(device) {
  case EV_SPI1: return HSPI;
  case EV_SPI2: return VSPI;
  default: return -1;
  }
}
  
/**
 * Initialize the hardware SPI device.
 * On the ESP32, hardware SPI is implemented via a set of default pins defined
 * as follows:
 *
 *
 */
void jshSPISetup(
    IOEventFlags device, //!< The identity of the SPI device being initialized.
    JshSPIInfo *inf      //!< Flags for the SPI device.
) {
  jsError(">> jshSPISetup device=%s, baudRate=%d, spiMode=%d, spiMSB=%d",
      jshGetDeviceString(device),
      inf->baudRate,
      inf->spiMode,
      inf->spiMSB);
  spi_t *spi = NULL;
  uint8_t dataMode = SPI_MODE0;
  switch(inf->spiMode) {
  case 0:
    dataMode = SPI_MODE0;
    break;
  case 1:
    dataMode = SPI_MODE1;
    break;
  case 2:
    dataMode = SPI_MODE2;
    break;
  case 3:
    dataMode = SPI_MODE3;
    break;
  }
  uint8_t bitOrder;
  if (inf->spiMSB == true) {
    bitOrder = SPI_MSBFIRST;
  } else {
    bitOrder = SPI_LSBFIRST;
  }
  int which_spi=getSPIFromDevice(device);
  if (which_spi == -1) {
	jsError( "Unexpected device for SPI initialization: %d", device);
	}
  else {
	int which_spi=getSPIFromDevice(device);
	Pin sck;
	Pin miso;
	Pin mosi;
	Pin ss; 
	if ( which_spi == HSPI ) {
	  sck = inf->pinSCK != PIN_UNDEFINED ? inf->pinSCK : 14;
	  miso = inf->pinMISO != PIN_UNDEFINED ? inf->pinMISO : 12;
	  mosi = inf->pinMOSI != PIN_UNDEFINED ? inf->pinMOSI : 13;
	  // Where do we get the SS pin?
	  //ss = inf->pinSS != PIN_UNDEFINED ? inf->pinSS : 15;
	  ss=15;
	}
	else {
	  sck = inf->pinSCK != PIN_UNDEFINED ? inf->pinSCK : 5;
	  miso = inf->pinMISO != PIN_UNDEFINED ? inf->pinMISO : 19;
	  mosi = inf->pinMOSI != PIN_UNDEFINED ? inf->pinMOSI : 23;
	  //ss = inf->pinSS != PIN_UNDEFINED ? inf->pinSS : 18;
	  ss=18;
	}
	spi=spiStartBus(which_spi, inf->baudRate, dataMode, bitOrder);  
    spiAttachSCK(spi, pinToESP32Pin(sck));
    spiAttachMISO(spi, pinToESP32Pin(miso));
    spiAttachMOSI(spi, pinToESP32Pin(mosi));
    spiAttachSS(spi, 0, pinToESP32Pin(ss));
    spiEnableSSPins(spi, 1<<0);
    spiSSEnable(spi);
	_spi[which_spi]=spi;
  }
}


/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(
    IOEventFlags device, //!< The identity of the SPI device through which data is being sent.
    int data             //!< The data to be sent or an indication that no data is to be sent.
) {
  int which_spi =getSPIFromDevice(device);
  if (which_spi == -1) {
	return -1;
  }
  //os_printf("> jshSPISend - device=%d, data=%x\n", device, data);
  int retData = (int)g_lastSPIRead;
  if (data >=0) {
    // Send 8 bits of data taken from "data" over the selected spi and store the returned
    // data for subsequent retrieval.
    spiTransferBits(_spi[which_spi], (uint32_t)data, &g_lastSPIRead, 8);
  } else {
    g_lastSPIRead = (uint32_t)-1;
  }
  return (int)retData;
}


/**
 * Send 16 bit data through the given SPI device.
 */
void jshSPISend16(
    IOEventFlags device, //!< Unknown
    int data             //!< Unknown
) {
  int which_spi=getSPIFromDevice(device);  
  spiWriteWord(_spi[which_spi], data);
}


/**
 * Set whether to send 16 bits or 8 over SPI.
 */
void jshSPISet16(
    IOEventFlags device, //!< Unknown
    bool is16            //!< Unknown
) {
  UNUSED(device);
  UNUSED(is16);
  jsError(">> jshSPISet16");
  jsError( "Not implemented");
  jsError("<< jshSPISet16");
}


/**
 * Wait until SPI send is finished.
 */
void jshSPIWait(
    IOEventFlags device //!< Unknown
) {
  UNUSED(device);
  int which_spi =getSPIFromDevice(device);  
  spiWaitReady(_spi[which_spi]);
}


/** Set whether to use the receive interrupt or not */
void jshSPISetReceive(IOEventFlags device, bool isReceive) {
  UNUSED(device);
  UNUSED(isReceive);
  jsError(">> jshSPISetReceive");
  jsError( "Not implemented");
  jsError("<< jshSPISetReceive");
}


