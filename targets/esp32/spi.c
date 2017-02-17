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
#include "spi.h"

#define UNUSED(x) (void)(x)

int getSPIChannelPnt(IOEventFlags device){
  return device - EV_SPI1;
}
void SPIChannelsInit(){
  int i;
  for(i = 0; i < SPIMax; i++){
    SPIChannels[i].spi = NULL;
    SPIChannels[i].spi_read = false;
	SPIChannels[i].g_lastSPIRead = (uint32_t)-1;
  }
  SPIChannels[0].HOST = HSPI_HOST;
  SPIChannels[1].HOST = VSPI_HOST;
}

void jshSetDeviceInitialised(IOEventFlags device, bool isInit);

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
Test with ILI9341 works, but could be faster.
Espruino supports sendig byte by byte, no mass sending is supported.
*/
  
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

  int channelPnt = getSPIChannelPnt(device);
  Pin sck, miso, mosi;
  if(SPIChannels[channelPnt].HOST == HSPI_HOST){
    sck = inf->pinSCK != PIN_UNDEFINED ? inf->pinSCK : 14;
    miso = inf->pinMISO != PIN_UNDEFINED ? inf->pinMISO : 12;
    mosi = inf->pinMOSI != PIN_UNDEFINED ? inf->pinMOSI : 13;
  }
  else {
    sck = inf->pinSCK != PIN_UNDEFINED ? inf->pinSCK : 5;
    miso = inf->pinMISO != PIN_UNDEFINED ? inf->pinMISO : 19;
    mosi = inf->pinMOSI != PIN_UNDEFINED ? inf->pinMOSI : 23;
  }

  spi_bus_config_t buscfg={
        .miso_io_num=miso,
        .mosi_io_num=mosi,
        .sclk_io_num=sck,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1
    };
  // SPI_DEVICE_BIT_LSBFIRST  - test inf->spiMSB need to look at what values...
  uint32_t flags = 0;
  
  spi_device_interface_config_t devcfg={
        .clock_speed_hz=inf->baudRate,
        .mode=inf->spiMode,
        .spics_io_num= -1,               //set CS not used by driver
        .queue_size=7,      //We want to be able to queue 7 transactions at a time
		.flags=flags
    };
  if(SPIChannels[channelPnt].spi){
	spi_bus_remove_device(SPIChannels[channelPnt].spi);
	spi_bus_free(SPIChannels[channelPnt].HOST);
	jsWarn("spi was already in use, removed old assignment");
  }
  esp_err_t ret=spi_bus_initialize(SPIChannels[channelPnt].HOST, &buscfg, 1);
  assert(ret==ESP_OK);
  ret = spi_bus_add_device(SPIChannels[channelPnt].HOST, &devcfg, &SPIChannels[channelPnt].spi);
  assert(ret==ESP_OK);

  jshSetDeviceInitialised(device, true);
}

/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(
    IOEventFlags device, //!< The identity of the SPI device through which data is being sent.
    int data             //!< The data to be sent or an indication that no data is to be sent.
) {
  int channelPnt = getSPIChannelPnt(device);
  uint8_t byte = (uint8_t)data;
  //os_printf("> jshSPISend - device=%d, data=%x\n", device, data);
  int retData = (int)SPIChannels[channelPnt].g_lastSPIRead;
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
    ret=spi_device_transmit(SPIChannels[channelPnt].spi, &t);  //Transmit - blocks until result - need to change this?
	assert(ret == ESP_OK);
	SPIChannels[channelPnt].g_lastSPIRead=t.rx_data[0];
	
  } else {
    SPIChannels[channelPnt].g_lastSPIRead = (uint32_t)-1;
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
  int channelPnt = getSPIChannelPnt(device);
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
  int channelPnt = getSPIChannelPnt(device);
  //spiWaitReady(_spi[which_spi]);
  //jsError(">> jshSPIWait: Not implemented");  
}


/** Set whether to use the receive interrupt or not */
void jshSPISetReceive(IOEventFlags device, bool isReceive) {
  int channelPnt = getSPIChannelPnt(device);
  SPIChannels[channelPnt].spi_read = isReceive;  
}
