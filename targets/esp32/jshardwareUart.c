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
#include <jsdevices.h>

void initUart(int uart_num,uart_config_t uart_config,int txpin,int rxpin){
  uart_param_config(uart_num, &uart_config);   //Configure UART1 parameters
  uart_set_pin(uart_num, txpin, rxpin, -1, -1); //Set UART0 pins(TX: IO16, RX: IO17, RTS: IO18, CTS: IO19)
  uart_driver_install(uart_num, 1024, 1024, 10, NULL, 0);  //Install UART driver( We don't need an event queue here)
}

void initConsole(){
  uart_config_t uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 122,
  }; 
  initUart(uart_console,uart_config,-1,-1);  
}

void consoleToEspruino(){
  uint8_t rxbuf[256];
  int len = uart_read_bytes(uart_console, rxbuf, sizeof(rxbuf), 100);  //Read data from UART
  if(len > 0){
	jshPushIOCharEvents(EV_SERIAL1, rxbuf, len);	  
  }	
}
