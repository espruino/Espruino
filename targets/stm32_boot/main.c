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
 * Bootloader entry point
 * ----------------------------------------------------------------------------
 */
#include "platform_config.h"
#include "utils.h"

#define BOOTLOADER_MAJOR_VERSION 0
#define BOOTLOADER_MINOR_VERSION 20

#define CMD_GET (0x00)
#define CMD_GET_ID (0x02)
#define CMD_READ (0x11)
#define CMD_WRITE (0x31)
#define CMD_ERASE (0x43)

#define FLASH_START 0x08000000

#define ACK (0x79)
#define NACK (0x1F)

typedef enum {
  BLS_UNDEFINED,
  BLS_INITED,             // Has got 0x7F byte...
  BLS_COMMAND_FIRST_BYTE, // Got first byte of command - waiting for inverted byte
  BLS_EXPECT_DATA,
} BootloaderState;

void setLEDs(int l) {
  jshPinOutput(LED1_PININDEX, l&1);
  jshPinOutput(LED2_PININDEX, (l>>1)&1);
  jshPinOutput(LED3_PININDEX, (l>>2)&1);
}

int main(void){

  initHardware();

  int flashy = 0;
  BootloaderState state = BLS_UNDEFINED;
  char currentCommand = 0;

  while (1) {
    if (!jshIsUSBSERIALConnected()) {
      jshPinOutput(LED2_PININDEX, 0);
      // reset, led off
    } else {
      jshPinOutput(LED3_PININDEX, ((flashy++)&0xFF)<5);
      // flash led
      int d = getc();
      if (d>=0) { // if we have data
        if (state==BLS_EXPECT_DATA) {

        } else if (state==BLS_INITED) {
          currentCommand = d;
          state = BLS_COMMAND_FIRST_BYTE;
        } else if (state==BLS_COMMAND_FIRST_BYTE) {
          if (currentCommand == d^0xFF) {
            unsigned int addr,i;
            char chksum, buffer[256];
            unsigned char nBytesMinusOne, nPages;
            // confirmed
            switch (currentCommand) {
            case CMD_GET: // get bootloader info
              putc(ACK);
              putc(5); // 6 bytes
              // now report what we support
              putc(BOOTLOADER_MAJOR_VERSION<<4 | BOOTLOADER_MINOR_VERSION); // Bootloader version
              // list supported commands
              putc(CMD_GET);
              putc(CMD_GET_ID);
              putc(CMD_READ);
              putc(CMD_WRITE);
              putc(CMD_ERASE); // erase
              putc(ACK); // last byte
              break;
            case CMD_GET_ID: // get chip ID
              putc(ACK);
              putc(1); // 2 bytes
              // now report what we support
              putc(0x04);
              // 0x30 F1 XL density
              // 0x14 F1 high density
              putc(0x30); // TODO: really?
              putc(ACK); // last byte
              break;
            case CMD_READ: // read memory
              putc(ACK);
              addr = (unsigned char)getc_blocking() << 24;
              addr |= (unsigned char)getc_blocking()  << 16;
              addr |= (unsigned char)getc_blocking()  << 8;
              addr |= (unsigned char)getc_blocking();
              chksum = getc_blocking();
              // TODO: check checksum
              putc(ACK);
              nBytesMinusOne = getc_blocking();
              chksum = getc_blocking();
              // TODO: check checksum
              putc(ACK);
              for (i=0;i<=nBytesMinusOne;i++)
                putc(((unsigned char*)addr)[i]);
              break;
            case CMD_WRITE: // write memory
              putc(ACK);
              addr = (unsigned char)getc_blocking() << 24;
              addr |= (unsigned char)getc_blocking()  << 16;
              addr |= (unsigned char)getc_blocking()  << 8;
              addr |= (unsigned char)getc_blocking();
              chksum = getc_blocking();
              // TODO: check checksum and address&3==0
              putc(ACK);
              setLEDs(4); // blue = wait for data
              nBytesMinusOne = getc_blocking();
              for (i=0;i<=nBytesMinusOne;i++)
                buffer[i] = getc_blocking();
              chksum = getc_blocking();
              setLEDs(1); // red =  write
              // TODO: check checksum and (nBytesMinusOne+1)&3==0
              FLASH_UnlockBank1();
              for (i=0;i<=nBytesMinusOne;i+=4) {
                FLASH_ProgramWord(addr+i, *(unsigned int*)&buffer[i]);
              }
              FLASH_LockBank1();
              setLEDs(0); // off
              putc(ACK);
              break;
            case CMD_ERASE: // erase memory
              putc(ACK);
              nPages = getc_blocking();
              if (nPages == 0xFF) {
                // all pages (except us!)
                setLEDs(1); // red =  write
                FLASH_UnlockBank1();
                for (i=16384;i<FLASH_TOTAL;i+=FLASH_PAGE_SIZE)
                  FLASH_ErasePage((uint32_t)(FLASH_START + i));
                FLASH_LockBank1();
                setLEDs(0); // off
                putc(ACK);
              } else {
                putc(NACK); // not implemented
              }
              break;
            default: // unknown command
              putc(NACK);
              break;
            }
          } else {
            // not correct
            putc(NACK);
          }
          state = BLS_INITED;
        } else {
          switch (d) {
          case 0x7F: // initialisation byte
                     putc(state == BLS_UNDEFINED ? ACK : NACK);
                     state = BLS_INITED;
                     break;
          }
        }
      }
    }
  }
}

