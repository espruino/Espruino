/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2021 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Low-level bootloader flash tool
 * ----------------------------------------------------------------------------
 */

#ifdef ESPR_BOOTLOADER_SPIFLASH

#include "flash.h"
#include "platform_config.h"
#include "jsutils.h"
#include "hardware.h"
#include "lcd.h"
#include "crc32.h"

typedef struct {
  uint32_t address;
  uint32_t size;
  uint32_t CRC;
  uint32_t version;
} FlashHeader;

#define FLASH_HEADER_ADDRESS 0x00300000


/// Read data while sending 0
__attribute__( ( long_call, section(".data") ) ) static void spiFlashRead(unsigned char *rx, unsigned int len) {
  NRF_GPIO_PIN_CLEAR_FAST((uint32_t)pinInfo[SPIFLASH_PIN_MOSI].pin);
  for (unsigned int i=0;i<len;i++) {
    int result = 0;
    for (int bit=0;bit<8;bit++) {
      NRF_GPIO_PIN_SET_FAST((uint32_t)pinInfo[SPIFLASH_PIN_SCK].pin);
      result = (result<<1) | NRF_GPIO_PIN_READ_FAST((uint32_t)pinInfo[SPIFLASH_PIN_MISO].pin);
      NRF_GPIO_PIN_CLEAR_FAST((uint32_t)pinInfo[SPIFLASH_PIN_SCK].pin);
    }
    rx[i] = result;
  }
}

__attribute__( ( long_call, section(".data") ) ) static void spiFlashWrite(unsigned char *tx, unsigned int len) {
  for (unsigned int i=0;i<len;i++) {
    int data = tx[i];
    for (int bit=7;bit>=0;bit--) {
      NRF_GPIO_PIN_WRITE_FAST((uint32_t)pinInfo[SPIFLASH_PIN_MOSI].pin, (data>>bit)&1 );
      NRF_GPIO_PIN_SET_FAST((uint32_t)pinInfo[SPIFLASH_PIN_SCK].pin);
      NRF_GPIO_PIN_CLEAR_FAST((uint32_t)pinInfo[SPIFLASH_PIN_SCK].pin);
    }
  }
}

__attribute__( ( long_call, section(".data") ) ) static void spiFlashWriteCS(unsigned char *tx, unsigned int len) {
  NRF_GPIO_PIN_CLEAR_FAST((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
  spiFlashWrite(tx,len);
  NRF_GPIO_PIN_SET_FAST((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
}

static unsigned char spiFlashStatus() {
  unsigned char buf = 5;
  NRF_GPIO_PIN_CLEAR_FAST((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
  spiFlashWrite(&buf, 1);
  spiFlashRead(&buf, 1);
  NRF_GPIO_PIN_SET_FAST((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
  return buf;
}

void spiFlashInit() {
#ifdef SPIFLASH_PIN_WP
  nrf_gpio_pin_write_output((uint32_t)pinInfo[SPIFLASH_PIN_WP].pin, 0);
#endif
  nrf_gpio_pin_write_output((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin, 1);
  nrf_gpio_pin_write_output((uint32_t)pinInfo[SPIFLASH_PIN_MOSI].pin, 1);
  nrf_gpio_pin_write_output((uint32_t)pinInfo[SPIFLASH_PIN_SCK].pin, 1);
  nrf_gpio_cfg_input((uint32_t)pinInfo[SPIFLASH_PIN_MISO].pin, NRF_GPIO_PIN_PULLUP);
#ifdef SPIFLASH_PIN_RST
  nrf_gpio_pin_write_output((uint32_t)pinInfo[SPIFLASH_PIN_RST].pin, 0);
  nrf_delay_us(100);
  nrf_gpio_pin_write((uint32_t)pinInfo[SPIFLASH_PIN_RST].pin, 1);
#endif
  nrf_delay_us(100);
  // disable lock bits
  // wait for write enable
  unsigned char buf[2];
  int timeout = 1000;
  while (timeout-- && !(spiFlashStatus()&2)) {
    buf[0] = 6; // write enable
    spiFlashWriteCS(buf,1);
  }
  buf[0] = 1; // write status register
  buf[1] = 0;
  spiFlashWriteCS(buf,2);
}

__attribute__( ( long_call, section(".data") ) ) void spiFlashReadAddr(unsigned char *buf, uint32_t addr, uint32_t len) {
  unsigned char b[4];
  // Read
  b[0] = 0x03;
  b[1] = addr>>16;
  b[2] = addr>>8;
  b[3] = addr;
  NRF_GPIO_PIN_CLEAR_FAST((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
  spiFlashWrite(b,4);
  spiFlashRead((unsigned char*)buf,len);
  NRF_GPIO_PIN_SET_FAST((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
}

__attribute__( ( long_call, section(".data") ) ) void intFlashErase(uint32_t addr) {
  NRF_NVMC->CONFIG = 2;
  while(!NRF_NVMC->READY);
  NRF_NVMC->ERASEPAGE = addr;
  while(!NRF_NVMC->READY);
  NRF_NVMC->CONFIG = 0;
  while(!NRF_NVMC->READY);
}

__attribute__( ( long_call, section(".data") ) ) void intFlashWrite(uint32_t addr, unsigned char *data, uint32_t len) {
  while (len) {
    NRF_NVMC->CONFIG = 1;
    while(!NRF_NVMC->READY);
    *((uint32_t*)addr) = *((uint32_t*)data);
    while(!NRF_NVMC->READY);
    NRF_NVMC->CONFIG = 0;
    while(!NRF_NVMC->READY);
    addr += 4;
    data += 4;
    len -= 4;
  }
}

bool flashEqual(FlashHeader header) {
  unsigned char buf[256];
  unsigned int size = header.size;
  int inaddr = FLASH_HEADER_ADDRESS + sizeof(FlashHeader);
  int outaddr = header.address;
  while (size>0) {
    unsigned int l = size;
    if (l>sizeof(buf)) l=sizeof(buf);
    spiFlashReadAddr(buf, inaddr, l);
    for (unsigned int i=0;i<l;i++) {
      unsigned char d = ((unsigned char*)outaddr)[i];
      if (buf[i] != d) {
        lcd_print("DIFF AT ");
        lcd_print_hex(outaddr+i);
        lcd_println("");
        lcd_print_hex(buf[i]);
        lcd_print(" V ");
        lcd_print_hex(d);
        lcd_println("");
        return false;
      }
    }
    inaddr += l;
    outaddr += l;
    size -= l;
  }
  return true;
}

// Inline LCD calls for general SPI LCDs
__attribute__( ( long_call, section(".data") ) ) void xlcd_wr(int data) {
  for (int bit=7;bit>=0;bit--) {
    NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_SCK, 0 );
    NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_MOSI, ((data>>bit)&1) );
    NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_SCK, 1 );
  }
}

__attribute__( ( long_call, section(".data") ) ) void xlcd_rect(int x1,int y1, int x2, int y2) {
  NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_DC, 0); // command
  NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_CS, 0);
  xlcd_wr(0x2A);
  NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_CS, 1);
  NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_DC, 1); // data
  NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_CS, 0);
  xlcd_wr(0);
  xlcd_wr(x1);
  xlcd_wr(0);
  xlcd_wr(x2);
  NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_CS, 1);
  NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_DC, 0); // command
  NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_CS, 0);
  xlcd_wr(0x2B);
  NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_CS, 1);
  NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_DC, 1); // data
  NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_CS, 0);
  xlcd_wr(0);
  xlcd_wr(y1);
  xlcd_wr(0);
  xlcd_wr(y2);
  NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_CS, 1);
  NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_DC, 0); // command
  NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_CS, 0);
  xlcd_wr(0x2C);
  NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_CS, 1);
  NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_DC, 1); // data
  NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_CS, 0);
  int l = (x2+1-x1) * (y2+1-y1);
  for (int x=0;x<l*2;x++)
    xlcd_wr(0xFF);
  NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_CS,1);
}

__attribute__( ( long_call, section(".data") ) ) void flashDoUpdate(FlashHeader header) {
  unsigned char buf[256];
  int size, addr;

  __disable_irq(); // No IRQs - if we're updating bootloader it could really screw us up!

  int percent = 0;
  // Erase
  size = header.size;
  addr = header.address;
  while (size>0) {
    intFlashErase(addr);
    addr += 4096;
    size -= 4096;
    percent = (addr-header.address)*120/header.size;
    if (percent>120) percent=120;
    xlcd_rect(60,182,60+percent,188);
    NRF_WDT->RR[0] = 0x6E524635; // kick watchdog
  }
  // Write
  size = header.size;
  int inaddr = FLASH_HEADER_ADDRESS + sizeof(FlashHeader);
  int outaddr = header.address;
  while (size>0) {
    unsigned int l = size;
    if (l>sizeof(buf)) l=sizeof(buf);
    spiFlashReadAddr(buf, inaddr, l);
    intFlashWrite(outaddr, buf, l);
    inaddr += l;
    outaddr += l;
    size -= l;
    percent = (outaddr-header.address)*120/header.size;
    if (percent>120) percent=120;
    xlcd_rect(60,192,60+percent,198);
    NRF_WDT->RR[0] = 0x6E524635; // kick watchdog
  }
  //flashEqual(header);
  // done!
  for (volatile int i=0;i<10000000;i++); // delay
  NVIC_SystemReset(); // reset!
}


void flashCheckAndRun() {
  spiFlashInit();
  FlashHeader header;
  spiFlashReadAddr((unsigned char *)&header, FLASH_HEADER_ADDRESS, sizeof(FlashHeader));
  if (header.address==0xFFFFFFFF) {
    // Not set - silently exit
    return;
  }
  lcd_println("FLASH HEADER");
  lcd_print_hex(header.address); lcd_println(" ADDR");
  lcd_print_hex(header.size); lcd_println(" SIZE");
  lcd_print_hex(header.CRC); lcd_println(" CRC");
  lcd_print_hex(header.version); lcd_println(" VERSION");
  // if (header.address==0xf7000) return; // NO BOOTLOADER - FOR TESTINGs
  // Calculate CRC
  unsigned char buf[256];
  int size = header.size;
  int inaddr = FLASH_HEADER_ADDRESS + sizeof(FlashHeader);
  uint32_t crc = 0;
  while (size>0) {
    unsigned int l = size;
    if (l>sizeof(buf)) l=sizeof(buf);
    spiFlashReadAddr(buf, inaddr, l);
    crc = crc32_compute(buf, l, &crc);
    inaddr += l;
    size -= l;
  }
  if (crc != header.CRC) {
    // CRC is wrong - exits
    lcd_println("CRC MISMATCH");
    lcd_print_hex(crc); lcd_println("");lcd_println("");
    nrf_delay_us(1000000);
    return;
  }
  // All ok - check we haven't already flashed this!
  if (!flashEqual(header)) {
    lcd_println("BINARY DIFF. FLASHING...");
    xlcd_rect(60,180,180,180);
    xlcd_rect(60,190,180,190);
    xlcd_rect(60,200,180,200);

    flashDoUpdate(header);
  } else {
    lcd_println("BINARY MATCHES.");
  }
}

#endif
