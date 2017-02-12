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

bool serial2_initialized = false;
bool serial3_initialized = false;

void initUart(int uart_num,uart_config_t uart_config,int txpin,int rxpin){
  int r;
  r = uart_param_config(uart_num, &uart_config);   //Configure UART1 parameters
  r = uart_set_pin(uart_num, txpin, rxpin, -1, -1); //Set UART0 pins(TX: IO16, RX: IO17, RTS: IO18, CTS: IO19)
  r = uart_driver_install(uart_num, 1024, 1024, 10, NULL, 0);  //Install UART driver( We don't need an event queue here)
}

void initSerial(IOEventFlags device,JshUSARTInfo *inf){
  uart_config_t uart_config = {
	.baud_rate = inf->baudRate,
	.data_bits = (inf->bytesize == 7)? UART_DATA_7_BITS : UART_DATA_8_BITS,
	.stop_bits = (inf->stopbits == 1)? UART_STOP_BITS_1 : UART_STOP_BITS_2,
	//.flow_ctrl = (inf->xOnXOff)? UART_HW_FLOWCTRL_DISABLE : UART_HW_FLOWCTRL_CTS_RTS,
	.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
	.rx_flow_ctrl_thresh = 122,
	.parity = UART_PARITY_DISABLE
  };
  switch(inf->parity){
	case 0: uart_config.parity = UART_PARITY_DISABLE; break;
	case 1: uart_config.parity = UART_PARITY_ODD; break;
	case 2: uart_config.parity = UART_PARITY_EVEN; break;
  }
  if(device == EV_SERIAL1) initUart(uart_console,uart_config,-1,-1);
  else{
	if(device == EV_SERIAL2){
	  if(inf->pinTX == 0xff) inf->pinTX = 4;
	  if(inf->pinRX == 0xff) inf->pinRX = 5;
	  if(serial2_initialized) uart_driver_delete(uart_Serial2);
      initUart(uart_Serial2,uart_config,inf->pinTX,inf->pinRX);
	  serial2_initialized = true;
	}
	else{
	  if(inf->pinTX == 0xff) inf->pinTX = 17;
	  if(inf->pinRX == 0xff) inf->pinRX = 16;
	  if(serial3_initialized)   uart_driver_delete(uart_Serial3);
      initUart(uart_Serial3,uart_config,inf->pinTX,inf->pinRX);
	  serial3_initialized = true;
	}
  }
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

uint8_t rxbuf[256];
void consoleToEspruino(){
  int len = uart_read_bytes(uart_console, rxbuf, sizeof(rxbuf), 100);  //Read data from UART
  if(len > 0) jshPushIOCharEvents(EV_SERIAL1, rxbuf, len);	  	
}

void serialToEspruino(){
  int len;
  if(serial2_initialized){
    len = uart_read_bytes(uart_Serial2,rxbuf, sizeof(rxbuf),0);
    if(len > 0)jshPushIOCharEvents(EV_SERIAL2, rxbuf, len);
  }
  if(serial3_initialized){
    len = uart_read_bytes(uart_Serial3,rxbuf, sizeof(rxbuf),0);
    if(len > 0) jshPushIOCharEvents(EV_SERIAL3, rxbuf,len);
  }
}

void writeSerial(IOEventFlags device,uint8_t c){
  char str[2]; int r;
  str[1] = '\0';
  str[0] = (char)c;
  if(device == EV_SERIAL2){ r = uart_write_bytes(uart_Serial2, (const char*)str,1);}
  else{r = uart_write_bytes(uart_Serial3, (const char*)str,1);}
}
