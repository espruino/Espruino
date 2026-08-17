/*
 * This file is designed to support FREERTOS functions in Espruino,
 * a JavaScript interpreter for Microcontrollers designed by Gordon Williams
 *
 * Copyright (C) 2016 by Juergen Marsch
 *
 * This Source Code Form is subject to the terms of the Mozilla Publici
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains ESP32 board specific functions.
 * ----------------------------------------------------------------------------
 */

#include "jshardwareUart.h"
#include "driver/uart.h"
#include <stdio.h>
#include <string.h>
#include "jsdevices.h"
#include "jsinteractive.h"

#define uart_Serial1 0
#define uart_Serial2 1
#define uart_Serial3 2

#ifdef ESPR_USE_USB_SERIAL_JTAG
#include "driver/usb_serial_jtag.h"
#if ESP_IDF_VERSION_MAJOR >= 5
#include <unistd.h>
#else // is_connected not in IDF 4, so make our own
#include "hal/usb_serial_jtag_ll.h"
uint16_t usb_serial_jtag_idle_counter = 0;
uint32_t usb_serial_jtag_last_frame_counter = 0;
bool usb_serial_initialised = false;
bool usb_serial_jtag_is_driver_installed() {
  return usb_serial_initialised;
}
bool usb_serial_jtag_is_connected() {
  return usb_serial_jtag_idle_counter < 1000;
}
#endif

volatile bool usbUARTIsNotFlushed;
#endif

void jshSetDeviceInitialised(IOEventFlags device, bool isInit);

void initUart(int uart_num, uart_config_t uart_config, int txpin, int rxpin){
  esp_err_t err;

  err = uart_param_config(uart_num, &uart_config);
  ESP_ERROR_CHECK(err);
  err = uart_set_pin(uart_num, txpin, rxpin, -1, -1);
  ESP_ERROR_CHECK(err);
  err = uart_driver_install(uart_num, 1024, 1024, 0, NULL, 0);
  ESP_ERROR_CHECK(err);
}

void uninitSerial(IOEventFlags device) {
  if(device == EV_SERIAL1) {
    if (jshIsDeviceInitialised(EV_SERIAL1))
      uart_driver_delete(uart_Serial1);
    jshSetDeviceInitialised(EV_SERIAL1, false);
  } else if(device == EV_SERIAL2){
    if (jshIsDeviceInitialised(EV_SERIAL2))
      uart_driver_delete(uart_Serial2);
    jshSetDeviceInitialised(EV_SERIAL2, false);
#if ESPR_USART_COUNT>2
  } else if(device == EV_SERIAL3){
    if (jshIsDeviceInitialised(EV_SERIAL3))
      uart_driver_delete(uart_Serial3);
    jshSetDeviceInitialised(EV_SERIAL3, false);
#endif
  }
}

void initSerial(IOEventFlags device, JshUSARTInfo *inf){
  uart_config_t uart_config = {
    .baud_rate = inf->baudRate,
    .data_bits = (inf->bytesize == 7) ? UART_DATA_7_BITS : UART_DATA_8_BITS,
    .stop_bits = (inf->stopbits == 1) ? UART_STOP_BITS_1 : UART_STOP_BITS_2,
    .parity = UART_PARITY_DISABLE,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 122,
  };
  switch(inf->parity){
    case 0: uart_config.parity = UART_PARITY_DISABLE; break;
    case 1: uart_config.parity = UART_PARITY_ODD; break;
    case 2: uart_config.parity = UART_PARITY_EVEN; break;
  }
  uninitSerial(device);
  if(device == EV_SERIAL1) {
    initUart(uart_Serial1, uart_config, -1, -1); // FIXME: pins?
    jshSetFlowControlEnabled(device, inf->xOnXOff, inf->pinCTS);
    jshSetDeviceInitialised(EV_SERIAL1, true);
  } else if(device == EV_SERIAL2){
    if(inf->pinTX == 0xFF) inf->pinTX = 4; // FIXME: use what's in jspininfo
    if(inf->pinRX == 0xFF) inf->pinRX = 5;
    initUart(uart_Serial2, uart_config, inf->pinTX, inf->pinRX);
    jshSetFlowControlEnabled(device, inf->xOnXOff, inf->pinCTS);
    jshSetDeviceInitialised(EV_SERIAL2, true);
#if ESPR_USART_COUNT>2
  } else if(device == EV_SERIAL3){
    if(inf->pinTX == 0xFF) inf->pinTX = 17; // FIXME: use what's in jspininfo
    if(inf->pinRX == 0xFF) inf->pinRX = 16;
    initUart(uart_Serial3, uart_config, inf->pinTX, inf->pinRX);
    jshSetFlowControlEnabled(device, inf->xOnXOff, inf->pinCTS);
    jshSetDeviceInitialised(EV_SERIAL3, true);
#endif
  }
}



void initConsole() {
  bool needUART = true;
#ifdef ESPR_USE_USB_SERIAL_JTAG
  /* Configure USB-CDC driver */
  usb_serial_jtag_driver_config_t usb_serial_config = {
    .tx_buffer_size = 128,
    .rx_buffer_size = 128
  };
#if ESP_IDF_VERSION_MAJOR>=5
  if (!usb_serial_jtag_is_driver_installed()) {
#else // IDF <5
  usb_serial_initialised = true;
  usb_serial_jtag_idle_counter = 65535;
  usb_serial_jtag_last_frame_counter = USB_SERIAL_JTAG.fram_num.val;
  if (true) {
#endif
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_config));
  }
  jshSetDeviceInitialised(EV_USBSERIAL, true); // No need for USB flow control - backpressure is fine
  vTaskDelay(pdMS_TO_TICKS(1000)); // Now wait one second to allow USB to init if it's going to

#if ESP_IDF_VERSION_MAJOR>=5
  bool usbConnected = usb_serial_jtag_is_connected();
#else // IDF <5
  bool usbConnected = usb_serial_jtag_last_frame_counter != USB_SERIAL_JTAG.fram_num.val;
#endif
  if (usbConnected) {
    needUART = false;
#if ESP_IDF_VERSION_MAJOR<5
    usb_serial_jtag_idle_counter = 0; // ensure isconnected return true
#endif
  } else {
    usb_serial_jtag_driver_uninstall();
#if ESP_IDF_VERSION_MAJOR<5
    usb_serial_initialised = false;
#endif
  }
#endif
  // if default console is USB serial, we're not going to Serial1 so don't enable it
  if (DEFAULT_CONSOLE_DEVICE == EV_USBSERIAL)
    needUART = false;

  if (needUART) {
    uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 122,
    };
    initUart(uart_Serial1, uart_config, -1, -1);
    // Don't use hardware flow control on most ESP32 boards. CTS is not connected on most boards, so XON/XOFF is best
    jshSetFlowControlEnabled(EV_SERIAL1, true, PIN_UNDEFINED);
    jshSetDeviceInitialised(EV_SERIAL1, true);
  }
  vTaskDelay(pdMS_TO_TICKS(50)); // wait a bit for the UART to settle
  jsiOneSecondAfterStartup(); // now push to the correct UART
}



uint8_t rxbuf[256];
void pollSerialDevices() {
  // Handle transmission
#ifdef ESPR_USE_USB_SERIAL_JTAG
#if ESP_IDF_VERSION_MAJOR < 5 // hack to work out if we're connected or not on IDF4
  if (usb_serial_initialised) {
    uint32_t frame = USB_SERIAL_JTAG.fram_num.val;
    if (frame != usb_serial_jtag_last_frame_counter) {
      usb_serial_jtag_idle_counter = 0;
      usb_serial_jtag_last_frame_counter = frame;
    } else if (usb_serial_jtag_idle_counter<65535) usb_serial_jtag_idle_counter++;
  }
#endif
    /* The USB CDC UART on the C3 only writes the data to USB after a newline.
    We don't want that, so we call flush in this uart task if any data has been sent. */
  if (usb_serial_jtag_is_connected() && usbUARTIsNotFlushed) {
#if ESP_IDF_VERSION_MAJOR >= 5
      usb_serial_jtag_wait_tx_done(pdMS_TO_TICKS(10));
#else
      usb_serial_jtag_ll_txfifo_flush();
#endif
      usbUARTIsNotFlushed = false;
    }
#endif
  // Hand Receive
  if (jshGetIOCharEventsFree() < 256) { // if we don't have enough space for data
    jshHadEvent(); // ensure we wake up the main task
    vTaskDelay(pdMS_TO_TICKS(10)); // wait for the espruino task
    return; // don't bother reading if we can't put the data in
  }
  // sleep handling - if idle for 5s, we start waiting 100ms for data - otherwise just 1ms
  static uint16_t idleCount = 0;
  TickType_t ticksToWait = (idleCount>5000) ? pdMS_TO_TICKS(100) : pdMS_TO_TICKS(5);
  bool busy = false;
  int len;
  /* FIXME: we should use esp_vfs_usb_serial_jtag_use_driver/esp_vfs_dev_uart_register
  and then 'select' on ALL of these open files - then we can sleep for however long
  we want. */
#ifdef ESPR_USE_USB_SERIAL_JTAG
  if (usb_serial_jtag_is_connected()) {
    len = usb_serial_jtag_read_bytes(rxbuf, sizeof(rxbuf), ticksToWait);
    ticksToWait = 0;
    if(len > 0) {
      jshPushIOCharEvents(EV_USBSERIAL, rxbuf, len);
      busy = true;
    }
  }
#endif
  if(jshIsDeviceInitialised(EV_SERIAL1)){
    len = uart_read_bytes(uart_Serial1, rxbuf, sizeof(rxbuf), ticksToWait);
    ticksToWait = 0;
    if(len > 0) {
      jshPushIOCharEvents(EV_SERIAL1, rxbuf, len);
      busy = true;
    }
  }
  if(jshIsDeviceInitialised(EV_SERIAL2)){
    len = uart_read_bytes(uart_Serial2, rxbuf, sizeof(rxbuf), ticksToWait);
    ticksToWait = 0;
    if(len > 0) {
      jshPushIOCharEvents(EV_SERIAL2, rxbuf, len);
      busy = true;
    }
  }
#if ESPR_USART_COUNT>2
  if(jshIsDeviceInitialised(EV_SERIAL3)){
    len = uart_read_bytes(uart_Serial3, rxbuf, sizeof(rxbuf), 0/*don't wait*/);
    if(len > 0) {
      jshPushIOCharEvents(EV_SERIAL3, rxbuf, len);
      busy = true;
    }
  }
#endif
  // idle counter
  if (busy) idleCount = 0;
  else if (idleCount<65535) idleCount++;
}

void writeSerial(IOEventFlags device, uint8_t c){
  char str[2] = { (char)c, '\0' };
#ifdef ESPR_USE_USB_SERIAL_JTAG
  if(device == EV_USBSERIAL) {
    usb_serial_jtag_write_bytes(&c, 1, 0);
    usbUARTIsNotFlushed = true; // ensure we flash USB eventually
  }
#endif
  if(device == EV_SERIAL1 && jshIsDeviceInitialised(EV_SERIAL1)){
    uart_write_bytes(uart_Serial1, str, 1);
  }
  if(device == EV_SERIAL2 && jshIsDeviceInitialised(EV_SERIAL2)){
    uart_write_bytes(uart_Serial2, str, 1);
  }
#if ESPR_USART_COUNT>2
  else if(device == EV_SERIAL3 && jshIsDeviceInitialised(EV_SERIAL3)){
    uart_write_bytes(uart_Serial3, str, 1);
  }
#endif
}
