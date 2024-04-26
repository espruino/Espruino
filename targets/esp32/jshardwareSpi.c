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
#include "jshardwareSpi.h"

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
#if ESP_IDF_VERSION_5    
  SPIChannels[0].HOST = SPI2_HOST;
  SPIChannels[1].HOST = SPI3_HOST;
#else
  SPIChannels[0].HOST = HSPI_HOST;
  SPIChannels[1].HOST = VSPI_HOST;
#endif  
}
void SPIChannelReset(int channelPnt){
  spi_bus_remove_device(SPIChannels[channelPnt].spi);
  spi_bus_free(SPIChannels[channelPnt].HOST);
  SPIChannels[channelPnt].spi = NULL;
  SPIChannels[channelPnt].spi_read = false;
  SPIChannels[channelPnt].g_lastSPIRead = (uint32_t)-1;
}
void SPIReset(){
  int i;
  for(i = 0; i < SPIMax; i++){
    if(SPIChannels[i].spi != NULL) SPIChannelReset(i);
  }
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


volatile spi_transaction_t spi_trans;
volatile bool spi_Sending = false;

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
  int dma_chan = 0;
  Pin sck, miso, mosi;
#if ESP_IDF_VERSION_5   
  if(SPIChannels[channelPnt].HOST == SPI2_HOST){
#else
  if(SPIChannels[channelPnt].HOST == HSPI_HOST){
#endif  
    dma_chan = 1;
    sck = inf->pinSCK != PIN_UNDEFINED ? inf->pinSCK : 14;
    miso = inf->pinMISO != PIN_UNDEFINED ? inf->pinMISO : 12;
    mosi = inf->pinMOSI != PIN_UNDEFINED ? inf->pinMOSI : 13;
  }
  else {
     dma_chan = 2;
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
  SPIChannelReset(channelPnt);
  jsWarn("spi was already in use, removed old assignment");
  }
  esp_err_t ret=spi_bus_initialize(SPIChannels[channelPnt].HOST, &buscfg, dma_chan);
  assert(ret==ESP_OK);
  ret = spi_bus_add_device(SPIChannels[channelPnt].HOST, &devcfg, &SPIChannels[channelPnt].spi);
  assert(ret==ESP_OK);

  jshSetDeviceInitialised(device, true);
}

/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(
    IOEventFlags device,
    int data
) {
  int channelPnt = getSPIChannelPnt(device);
  uint8_t byte = (uint8_t)data;
  if (data >=0) {
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length=8;
    t.tx_buffer=&data;
    t.flags=SPI_TRANS_USE_RXDATA;
    ret=spi_device_transmit(SPIChannels[channelPnt].spi, &t);
    assert(ret == ESP_OK);
    SPIChannels[channelPnt].g_lastSPIRead=t.rx_data[0];
  } else {
    SPIChannels[channelPnt].g_lastSPIRead = (uint32_t)-1;
  }
  int retData = (int)SPIChannels[channelPnt].g_lastSPIRead;
  return (int)retData;
}

/** Send data in tx through the given SPI device and return the response in
 * rx (if supplied). Returns true on success.
 */
bool jshSPISendMany(IOEventFlags device, unsigned char *tx, unsigned char *rx, size_t count, void (*callback)()) {
    if (!jshIsDeviceInitialised(device)) return false;
    if (count==1) {
      int r = jshSPISend(device, tx?*tx:-1);
      if (rx) *rx = r;
      if(callback)callback();
      return true;
    }
  jshSPIWait(device);
    int channelPnt = getSPIChannelPnt(device);
  esp_err_t ret;
    memset(&spi_trans, 0, sizeof(spi_trans));
    spi_trans.length=count*8;
    spi_trans.tx_buffer=tx;
    spi_trans.rx_buffer=rx;
  spi_Sending = true;
    ret=spi_device_queue_trans(SPIChannels[channelPnt].spi, &spi_trans, rx?0:portMAX_DELAY);

  if (ret != ESP_OK) {
    spi_Sending = false;
      jsExceptionHere(JSET_INTERNALERROR, "SPI Send Error %d", ret);
      return false;
    }
  jshSPIWait(device);
  if(callback)callback();
  return true;
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
void jshSPIWait(IOEventFlags device) {
  int channelPnt = getSPIChannelPnt(device);
  if(!spi_Sending)return;
  esp_err_t ret;
  ret=spi_device_get_trans_result(SPIChannels[channelPnt].spi, &spi_trans, portMAX_DELAY);
  if (ret != ESP_OK) {
    jsExceptionHere(JSET_INTERNALERROR, "SPI Send Error %d", ret);
  }
  spi_Sending = false;
  }


/** Set whether to use the receive interrupt or not */
void jshSPISetReceive(IOEventFlags device, bool isReceive) {
  int channelPnt = getSPIChannelPnt(device);
  SPIChannels[channelPnt].spi_read = isReceive;
}
