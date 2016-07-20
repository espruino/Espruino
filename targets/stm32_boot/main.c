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
#ifdef LED3_PININDEX
  jshPinOutput(LED3_PININDEX, (l>>2)&1);
#else
  if (l&4) l|=3;
#endif
  jshPinOutput(LED1_PININDEX, l&1);
  jshPinOutput(LED2_PININDEX, (l>>1)&1);
}

int main(void) {
  initHardware();
  setLEDs(0b11);
  int flashy = 0;
  BootloaderState state = BLS_UNDEFINED;
  char currentCommand = 0;
  
  unsigned int buttonLifted = 0;
  unsigned int buttonPressed = 0;

  while (1) {
    // if we pressed the button to enter the bootloader, then released,
    // then pressed again (with debounce) then let's jump back to Espruino
    if (isButtonPressed()) {
      if (buttonPressed<0xFFFFFFFF) 
        buttonPressed++;
      if (buttonLifted>10000 && buttonPressed>10000) {
        setLEDs(0);
        jumpToEspruinoBinary();
      }
    } else {
      if (buttonLifted<0xFFFFFFFF) 
        buttonLifted++;
    }

    if (!jshIsUSBSERIALConnected()) {
      setLEDs(0b0101);
      // reset, led off
    } else {
      int f = (flashy>>9) & 0x7F;
      if (f&0x40) f=128-f;
      bool ledState = (((flashy++)&0xFF)<f);
#ifdef LED3_PININDEX
      setLEDs(ledState ? 4 : 0); // glow blue
#else
      setLEDs(ledState? ((flashy&0x10000)?1:2) : 0); // glow red/green
#endif

      // flash led
      int d = _getc();
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
              _putc(ACK);
              _putc(6); // 7 bytes
              // now report what we support
              _putc(BOOTLOADER_MAJOR_VERSION<<4 | BOOTLOADER_MINOR_VERSION); // Bootloader version
              // list supported commands
              _putc(CMD_GET);
              _putc(CMD_GET_ID);
              _putc(CMD_READ);
              _putc(CMD_GO);
              _putc(CMD_WRITE);
              _putc(CMD_EXTERASE); // erase
              _putc(ACK); // last byte
              break;
            case CMD_GET_ID: // get chip ID
              _putc(ACK);
              _putc(1); // 2 bytes
              // AN2606 - STM32 microcontroller system memory boot mode
              // http://www.st.com/st-web-ui/static/active/jp/resource/technical/document/application_note/CD00167594.pdf
              // 0x430 F1 XL density
              // 0x414 F1 high density
              // 0x6433 F401 CD
              // 0x6431 F411
#if defined(STM32F401)
              _putc(0x64); _putc(0x33);
#elif defined(STM32F411)
              _putc(0x64); _putc(0x31);
#else
              _putc(0x04); _putc(0x14);
#endif
              _putc(ACK); // last byte
              break;
            case CMD_READ: // read memory
              _putc(ACK);
              addr = _getc_blocking() << 24;
              addr |= _getc_blocking()  << 16;
              addr |= _getc_blocking()  << 8;
              addr |= _getc_blocking();
              chksum = _getc_blocking();
              // TODO: check checksum
              _putc(ACK);
              setLEDs(2); // green = wait for data
              nBytesMinusOne = _getc_blocking();
              chksum = _getc_blocking();
              // TODO: check checksum
              _putc(ACK);
              for (i=0;i<=nBytesMinusOne;i++)
                _putc(((unsigned char*)addr)[i]);
              setLEDs(0); // off
              flashy = 0; // reset glowing
              break;
            case CMD_GO: // read memory
              _putc(ACK);
              addr = _getc_blocking() << 24;
              addr |= _getc_blocking()  << 16;
              addr |= _getc_blocking()  << 8;
              addr |= _getc_blocking();
              chksum = _getc_blocking();
              // TODO: check checksum
              _putc(ACK);
              setLEDs(7); // jumping...
              unsigned int *ResetHandler = (unsigned int *)(addr + 4);
              void (*startPtr)() = *ResetHandler;
              startPtr();
              break;
            case CMD_WRITE: // write memory
              _putc(ACK);
              addr = _getc_blocking() << 24;
              addr |= _getc_blocking()  << 16;
              addr |= _getc_blocking()  << 8;
              addr |= _getc_blocking();
              chksumc = ((addr)&0xFF)^((addr>>8)&0xFF)^((addr>>16)&0xFF)^((addr>>24)&0xFF);
              chksum = _getc_blocking();
              if (chksumc != chksum) {
                _putc(NACK);
                break;
              }
              _putc(ACK);
              setLEDs(2); // green = wait for data
              nBytesMinusOne = _getc_blocking();
              chksumc = nBytesMinusOne;
              for (i=0;i<=nBytesMinusOne;i++) {
                buffer[i] = _getc_blocking();
                chksumc = chksumc^buffer[i];
              }
              chksum = _getc_blocking(); // FIXME found to be stalled here
              setLEDs(1); // red = write
              if (chksumc != chksum || (nBytesMinusOne+1)&3!=0) {
                _putc(NACK);
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
              flashy = 0; // reset glowing
              _putc(ACK); //  TODO - could speed up writes by ACKing beforehand if we have space
              break;
            case CMD_EXTERASE: // erase memory
              _putc(ACK);
              nPages = _getc_blocking() << 8;
              nPages |= _getc_blocking();
              chksum = _getc_blocking();
              // TODO: check checksum
              if (nPages == 0xFFFF) {
                // all pages (except us!)
                setLEDs(1); // red =  write
                #if defined(STM32F2) || defined(STM32F4)
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

                #if defined(STM32F2) || defined(STM32F4)
                  for (i=1;i<8;i++) { // might as well do all of them
                    setLEDs(1 << (i&1)); // R,G,R,G,...
                    FLASH_EraseSector(FLASH_Sector_0 + (FLASH_Sector_1-FLASH_Sector_0)*i, VoltageRange_3); // a FLASH_Sector_## constant
                  }
                  FLASH_Lock();
                #else
                  for (i=BOOTLOADER_SIZE;i<FLASH_TOTAL;i+=FLASH_PAGE_SIZE) {
                    setLEDs(1 << (i%3)); // R,G,B,etc
                    FLASH_ErasePage((uint32_t)(FLASH_START + i));
                  }
                  FLASH_LockBank1();
                #endif
                setLEDs(0); // off
                _putc(ACK);
              } else {
                _putc(NACK); // not implemented
              }
              break;
            default: // unknown command
              _putc(NACK);
              break;
            }
          } else {
            // not correct
            _putc(NACK);
          }
          state = BLS_INITED;
        } else {
          switch (d) {
          case 0x7F: // initialisation byte
                     _putc(state == BLS_UNDEFINED ? ACK : NACK);
                     state = BLS_INITED;
                     break;
          }
        }
      }
    }
  }
}

