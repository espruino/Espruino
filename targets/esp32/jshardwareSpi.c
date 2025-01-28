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
 * Contains ESP32 board specific functions to Support SPI Peripherials
 * 
 * Espruino models a given SPI peripherial interface in the SPIChannels[channelPnt] struct.
 *   With elements for each identified Espruino SPI Device.
 * 
 * The jshSPISetup() function:
 *  Initialises a target SPI Bus to an ESP32 'peripherial Host' (Espruino Device SPI1 or SPI2)
 *   - Assigns Pins etc via struct spi_bus_config_t
 *   - Stores the IDF host ID against an Espruino SPIChannel in SPIChannels[channelPnt].HOST 
 *  and registers an 'ESP32 Device' attached to the bus 
 *   - configures timing etc via struct dev_config
 *   - assigns handle (via pointer SPIChannels[channelPnt].spi) for sendingtransactions to device 
 * 
 * Note this SPIChannels[] model combines the one to many ESP32 Host/Bus and Device concepts into one Espruino Channel.
 * However Espruino channels do not include Bus CS pins (a descriminating element of an ESP32 Bus vs device)
 * Espruino identifies the CS pin from individual JS SPI.send commands and sets/clears it a higher level in jswrap_spi_send
 * ----------------------------------------------------------------------------
 */

#include "jshardwareSpi.h"
#include "driver/gpio.h"
#include "hal/spi_types.h"
#include "jshardware.h"
#include "jsinteractive.h"
#include "jspininfo.h"

#define UNUSED(x) (void)(x)

#if ESP_IDF_VERSION_MAJOR >= 4     // Modified for issue #2601
#define SPICHANNEL0_HOST SPI2_HOST // SPI1_host internal use only
#define SPICHANNEL1_HOST SPI3_HOST
#else // allow original ESP32 build in IDF V3
#define SPICHANNEL0_HOST HSPI_HOST
#define SPICHANNEL1_HOST VSPI_HOST
#endif

int getSPIChannelPnt(IOEventFlags device) { return device - EV_SPI1; }
void SPIChannelsInit() {
  int i;
  for (i = 0; i < SPIMax; i++) {
    SPIChannels[i].spi = NULL;
    SPIChannels[i].spi_read = false;
    SPIChannels[i].g_lastSPIRead = (uint32_t)-1;
  }
  SPIChannels[0].HOST = SPICHANNEL0_HOST;
#if SPIMax > 1
  SPIChannels[1].HOST = SPICHANNEL1_HOST;
#endif
}
void SPIChannelReset(int channelPnt) {
  spi_bus_remove_device(SPIChannels[channelPnt].spi);
  spi_bus_free(SPIChannels[channelPnt].HOST);
  SPIChannels[channelPnt].spi = NULL;
  SPIChannels[channelPnt].spi_read = false;
  SPIChannels[channelPnt].g_lastSPIRead = (uint32_t)-1;
  jsDebug(DBG_INFO, "SPIChannelReset: for channel:%d,  assigned to host device %d\n",
          channelPnt, SPIChannels[channelPnt].HOST);
}
void SPIReset() {
  int i;
  for (i = 0; i < SPIMax; i++) {
    if (SPIChannels[i].spi != NULL)
      SPIChannelReset(i);
  }
}
void jshSetDeviceInitialised(IOEventFlags device, bool isInit);

/*  Original Notes by @wilberforce removed (descrepencies now clarified) at fix
 * for issue #2601 - See previous versions)
 */

volatile spi_transaction_t spi_trans;
volatile bool spi_Sending = false;

void jshSPISetup(
    IOEventFlags device, //!< The identity of the SPI device being initialized.
    JshSPIInfo *inf      //!< Flags for the SPI device.
) {
  int channelPnt = getSPIChannelPnt(device);

  // Modified for issue #2601 - Start
  int dma_chan = 0;
  #if ESP_IDF_VERSION_MAJOR >= 4
    dma_chan = SPI_DMA_CH_AUTO;
  #else
    dma_chan = (SPIChannels[channelPnt].HOST == SPICHANNEL0_HOST) ? 1 : 2;
  #endif

  // Default pins as set in board.py 
  JshPinFunction funcType = jshGetPinFunctionFromDevice(device);
  if (!jshIsPinValid(inf->pinSCK))
    inf->pinSCK = jshFindPinForFunction(funcType, JSH_SPI_SCK);
  if (!jshIsPinValid(inf->pinMISO))
    inf->pinMISO = jshFindPinForFunction(funcType, JSH_SPI_MISO);
  if (!jshIsPinValid(inf->pinMOSI))
    inf->pinMOSI = jshFindPinForFunction(funcType, JSH_SPI_MOSI);

  #ifdef DEBUG
    char funcTypeStr[50];
    jshPinFunctionToString(funcType, JSPFTS_DEVICE | JSPFTS_DEVICE_NUMBER,
                          funcTypeStr, sizeof(funcTypeStr));
    jsDebug(DBG_INFO,
            "jshSPISetup: for host: %d, SPI pins on device: %s, identified as "
            "SCK: %d, MISO: %d, MISI: %d\n",
            SPIChannels[channelPnt].HOST, funcTypeStr, inf->pinSCK, inf->pinMISO,
            inf->pinMOSI);
  #endif
  // Modified for issue #2601 - End

  spi_bus_config_t buscfg = {.miso_io_num = inf->pinMISO,
                             .mosi_io_num = inf->pinMOSI,
                             .sclk_io_num = inf->pinSCK,
                             .quadwp_io_num = -1,
                             .quadhd_io_num = -1};
  // SPI_DEVICE_BIT_LSBFIRST  - test inf->spiMSB need to look at what values...
  uint32_t flags = 0;

  spi_device_interface_config_t devcfg = {
      .clock_speed_hz = inf->baudRate,
      .mode = inf->spiMode,
      .spics_io_num = -1, // set CS not used by driver (provided via spi.read JS statements)
      .queue_size = 7, // We want to be able to queue 7 transactions at a time
      .flags = flags};
  if (SPIChannels[channelPnt].spi) {
    SPIChannelReset(channelPnt);
    jsWarn("spi was already in use, removed old assignment");
  }
  esp_err_t ret =
      spi_bus_initialize(SPIChannels[channelPnt].HOST, &buscfg, dma_chan);
  assert(ret == ESP_OK);
  ret = spi_bus_add_device(SPIChannels[channelPnt].HOST, &devcfg,
                           &SPIChannels[channelPnt].spi);
  assert(ret == ESP_OK);

  jshSetDeviceInitialised(device, true);
}

/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(IOEventFlags device, int data) {
  int channelPnt = getSPIChannelPnt(device);
  uint8_t byte = (uint8_t)data;
  if (data >= 0) {
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.tx_buffer = &data;
    t.flags = SPI_TRANS_USE_RXDATA;
    ret = spi_device_transmit(SPIChannels[channelPnt].spi, &t);
    assert(ret == ESP_OK);
    SPIChannels[channelPnt].g_lastSPIRead = t.rx_data[0];
  } else {
    SPIChannels[channelPnt].g_lastSPIRead = (uint32_t)-1;
  }
  int retData = (int)SPIChannels[channelPnt].g_lastSPIRead;
  return (int)retData;
}

/** Send data in tx through the given SPI device and return the response in
 * rx (if supplied). Returns true on success.
 */
bool jshSPISendMany(IOEventFlags device, unsigned char *tx, unsigned char *rx,
                    size_t count, void (*callback)()) {
  if (!jshIsDeviceInitialised(device))
    return false;
  if (count == 1) {
    int r = jshSPISend(device, tx ? *tx : -1);
    if (rx)
      *rx = r;
    if (callback)
      callback();
    return true;
  }
  jshSPIWait(device);
  int channelPnt = getSPIChannelPnt(device);
  esp_err_t ret;
  memset(&spi_trans, 0, sizeof(spi_trans));
  spi_trans.length = count * 8;
  spi_trans.tx_buffer = tx;
  spi_trans.rx_buffer = rx;
  spi_Sending = true;
  ret = spi_device_queue_trans(SPIChannels[channelPnt].spi, &spi_trans,
                               rx ? 0 : portMAX_DELAY);
  if (ret != ESP_OK) {
    spi_Sending = false;
    jsExceptionHere(JSET_INTERNALERROR, "SPI Send Error %d", ret);
    return false;
  }
  jshSPIWait(device);
  if (callback)
    callback();
  return true;
}

/**
 * Send 16 bit data through the given SPI device.
 */
void jshSPISend16(IOEventFlags device, //!< Unknown
                  int data             //!< Unknown
) {
  int channelPnt = getSPIChannelPnt(device);
  // spiWriteWord(_spi[which_spi], data);
  jsError(">> jshSPISend16: Not implemented");
}

/**
 * Set whether to send 16 bits or 8 over SPI.
 */
void jshSPISet16(IOEventFlags device, //!< Unknown
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
  if (!spi_Sending)
    return;
  esp_err_t ret;
  ret = spi_device_get_trans_result(SPIChannels[channelPnt].spi, &spi_trans,
                                    portMAX_DELAY);
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
