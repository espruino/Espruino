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

#define BOOTLOADER_MAJOR_VERSION 3 // needed for Ext Erase in stm32loader.py
#define BOOTLOADER_MINOR_VERSION 0

#define CMD_GET (0x00)
#define CMD_GET_ID (0x02)
#define CMD_READ (0x11)
#define CMD_GO (0x21)
#define CMD_WRITE (0x31)
#define CMD_EXTERASE (0x44)

#define FLASH_START 0x08000000
#define RAM_START 0x20000000

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
#ifdef LED3_PININDEX
  jshPinOutput(LED3_PININDEX, (l>>2)&1);
#endif
}

int main(void) {
  initHardware();
  int flashy = 0;
  BootloaderState state = BLS_UNDEFINED;
  char currentCommand = 0;

  while (1) {
    if (!jshIsUSBSERIALConnected()) {
      setLEDs(0b0101);
      // reset, led off
    } else {
      int f = (flashy>>9) & 0x7F;
      if (f&0x40) f=128-f;
      setLEDs((((flashy++)&0xFF)<f) ? 4 : 0);

      // flash led
      int d = getc();
      if (d>=0) { // if we have data
        if (state==BLS_EXPECT_DATA) {

        } else if (state==BLS_INITED) {
          currentCommand = d;
          if (d!=255) state = BLS_COMMAND_FIRST_BYTE;
        } else if (state==BLS_COMMAND_FIRST_BYTE) {
          if (currentCommand == d^0xFF) {
            unsigned int addr,i;
            char chksum, chksumc, buffer[256];
            unsigned int nBytesMinusOne, nPages;
            // confirmed
            switch (currentCommand) {
            case CMD_GET: // get bootloader info
              putc(ACK);
              putc(6); // 7 bytes
              // now report what we support
              putc(BOOTLOADER_MAJOR_VERSION<<4 | BOOTLOADER_MINOR_VERSION); // Bootloader version
              // list supported commands
              putc(CMD_GET);
              putc(CMD_GET_ID);
              putc(CMD_READ);
              putc(CMD_GO);
              putc(CMD_WRITE);
              putc(CMD_EXTERASE); // erase
              putc(ACK); // last byte
              break;
            case CMD_GET_ID: // get chip ID
              putc(ACK);
              putc(1); // 2 bytes
              // 0x430 F1 XL density
              // 0x414 F1 high density
              putc(0x04); putc(0x14);
              putc(ACK); // last byte
              break;
            case CMD_READ: // read memory
              putc(ACK);
              addr = getc_blocking() << 24;
              addr |= getc_blocking()  << 16;
              addr |= getc_blocking()  << 8;
              addr |= getc_blocking();
              chksum = getc_blocking();
              // TODO: check checksum
              putc(ACK);
              setLEDs(2); // green = wait for data
              nBytesMinusOne = getc_blocking();
              chksum = getc_blocking();
              // TODO: check checksum
              putc(ACK);
              for (i=0;i<=nBytesMinusOne;i++)
                putc(((unsigned char*)addr)[i]);
              setLEDs(0); // off
              break;
            case CMD_GO: // read memory
              putc(ACK);
              addr = getc_blocking() << 24;
              addr |= getc_blocking()  << 16;
              addr |= getc_blocking()  << 8;
              addr |= getc_blocking();
              chksum = getc_blocking();
              // TODO: check checksum
              putc(ACK);
              setLEDs(7); // jumping...
              unsigned int *ResetHandler = (unsigned int *)(addr + 4);
              void (*startPtr)() = *ResetHandler;
              startPtr();
              break;
            case CMD_WRITE: // write memory
              putc(ACK);
              addr = getc_blocking() << 24;
              addr |= getc_blocking()  << 16;
              addr |= getc_blocking()  << 8;
              addr |= getc_blocking();
              chksumc = ((addr)&0xFF)^((addr>>8)&0xFF)^((addr>>16)&0xFF)^((addr>>24)&0xFF);
              chksum = getc_blocking();
              if (chksumc != chksum) {
                putc(NACK);
                break;
              }
              putc(ACK);
              setLEDs(2); // green = wait for data
              nBytesMinusOne = getc_blocking();
              chksumc = nBytesMinusOne;
              for (i=0;i<=nBytesMinusOne;i++) {
                buffer[i] = getc_blocking();
                chksumc = chksumc^buffer[i];
              }
              chksum = getc_blocking(); // FIXME found to be stalled here
              setLEDs(1); // red = write
              if (chksumc != chksum || (nBytesMinusOne+1)&3!=0) {
                putc(NACK);
                break;
              }
              if (addr>=FLASH_START && addr<RAM_START) {
                #ifdef STM32API2
                  FLASH_Unlock();
                #else
                  FLASH_UnlockBank1();
                #endif
                #if defined(STM32F2) || defined(STM32F4)
                  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
                #elif defined(STM32F3)
                  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
                #else
                  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
                #endif
                for (i=0;i<=nBytesMinusOne;i+=4) {
                  unsigned int realaddr = addr+i;
                  if (realaddr >= (FLASH_START+BOOTLOADER_SIZE)) // protect bootloader
                    FLASH_ProgramWord(realaddr, *(unsigned int*)&buffer[i]);
                }
                #ifdef STM32API2
                  FLASH_Lock();
                #else
                  FLASH_LockBank1();
                #endif
              } else { 
                // normal write
                for (i=0;i<=nBytesMinusOne;i+=4) {
                  unsigned int realaddr = addr+i;
                  *((unsigned int*)realaddr) = *(unsigned int*)&buffer[i];
                }
              }
                
              setLEDs(0); // off
              putc(ACK); //  TODO - could speed up writes by ACKing beforehand if we have space
              break;
            case CMD_EXTERASE: // erase memory
              putc(ACK);
              nPages = getc_blocking() << 8;
              nPages |= getc_blocking();
              chksum = getc_blocking();
              // TODO: check checksum
              if (nPages == 0xFFFF) {
                // all pages (except us!)
                setLEDs(1); // red =  write
                #ifdef STM32API2
                  FLASH_Unlock();
                #else
                  FLASH_UnlockBank1();
                #endif
                #if defined(STM32F2) || defined(STM32F4)
                  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
                #elif defined(STM32F3)
                  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
                #else
                  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
                #endif
                for (i=BOOTLOADER_SIZE;i<FLASH_TOTAL;i+=FLASH_PAGE_SIZE) {
                  setLEDs(1 << (i%3)); // R,G,B,etc
                  FLASH_ErasePage((uint32_t)(FLASH_START + i));
                }
                #ifdef STM32API2
                  FLASH_Lock();
                #else
                  FLASH_LockBank1();
                #endif
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

