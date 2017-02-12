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
#include "driver/spi_master.h"

#define UNUSED(x) (void)(x)

// Convert an Espruino pin to an ESP32 pin number.
gpio_num_t pinToESP32Pin(Pin pin);

static uint32_t  g_lastSPIRead = (uint32_t)-1;

// Use only one device at time for now.... Expand to support two devices in the future
spi_device_handle_t spi = NULL;
    
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

Does not correspond to:
https://github.com/espressif/esp-idf/blob/master/examples/26_spi_master/main/spi_master.c#L34

#define PIN_NUM_MISO 25
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  19
#define PIN_NUM_CS   22

#define PIN_NUM_DC   21
#define PIN_NUM_RST  18
#define PIN_NUM_BCKL 5

To do:
implement inf->spiMSB
Test!
*/

int getSPIFromDevice( IOEventFlags device  ) {
  switch(device) {
  case EV_SPI1: return HSPI_HOST;
  case EV_SPI2: return VSPI_HOST;
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

  int which_spi=getSPIFromDevice(device);
  if (which_spi == -1) {
    jsError( "Unexpected device for SPI initialization: %d", device);
    return;
  }
  Pin sck;
  Pin miso;
  Pin mosi;
  Pin ss; 
  if ( which_spi == HSPI_HOST ) {
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

  spi_bus_config_t buscfg={
        .miso_io_num=miso,
        .miso_io_num=mosi,
        .sclk_io_num=sck,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1
    };
  // SPI_DEVICE_BIT_LSBFIRST  - test inf->spiMSB need to look at what values...
  uint32_t flags = 0;
  
  spi_device_interface_config_t devcfg={
        .clock_speed_hz=inf->baudRate,
        .mode=inf->spiMode,
        .spics_io_num=ss,               //CS pin
        .queue_size=7,      //We want to be able to queue 7 transactions at a time
		.flags=flags
    };
  esp_err_t ret=spi_bus_initialize(which_spi, &buscfg, 1);
  assert(ret==ESP_OK);
  ret=spi_bus_add_device(which_spi, &devcfg, &spi);
  assert(ret==ESP_OK);
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
    //spiTransferBits(_spi[which_spi], (uint32_t)data, &g_lastSPIRead, 8);
	esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&data;               //The data is the cmd itself
	// https://esp-idf.readthedocs.io/en/latest/api/spi_master.html#type-definitions
	// should this be a switch or always read?
	t.flags=SPI_TRANS_USE_RXDATA;
    ret=spi_device_transmit(spi, &t);  //Transmit - blocks until result - need to change this?
	g_lastSPIRead=t.rx_data[0];
	
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
  //spiWriteWord(_spi[which_spi], data);
  jsError(">> jshSPISend16: Not implemented");
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
  jsError(">> jshSPISend16: Not implemented");
}


/**
 * Wait until SPI send is finished.
 */
void jshSPIWait(
    IOEventFlags device //!< Unknown
) {
  UNUSED(device);
  int which_spi =getSPIFromDevice(device);  
  //spiWaitReady(_spi[which_spi]);
  jsError(">> jshSPIWait: Not implemented");  
}


/** Set whether to use the receive interrupt or not */
void jshSPISetReceive(IOEventFlags device, bool isReceive) {
  UNUSED(device);
  UNUSED(isReceive);
  jsError(">> jshSPISetReceive: Not implemented");    
}
