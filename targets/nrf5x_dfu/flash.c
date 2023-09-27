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

/// Header inside file in flash storage
typedef struct {
  uint32_t address;
  uint32_t size;
  uint32_t CRC;
  uint32_t version;
} FlashHeader;

/// Structure for File Storage. It's important this is 8 byte aligned for platforms that only support 64 bit writes
typedef struct {
  uint32_t size; ///< Total size (and flags in the top 8 bits)
  char name[28]; ///< 0-padded filename
} JsfFileHeader;

#define JSF_START_ADDRESS 0 /* actual flash starts at 0 - espruino's one is memory mapped */
// Set up the end address of external flash
#ifdef FLASH_SAVED_CODE2_START
// if there's a second bank of flash to use, it means Bank 1 was INTERNAL flash, Bank 2 is external
#define JSF_END_ADDRESS (FLASH_SAVED_CODE_START+FLASH_SAVED_CODE2_LENGTH)
#else
#define JSF_END_ADDRESS (FLASH_SAVED_CODE_START+FLASH_SAVED_CODE_LENGTH)
#endif

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

__attribute__( ( long_call, section(".data") ) ) static unsigned char spiFlashStatus() {
  unsigned char buf = 5;
  NRF_GPIO_PIN_CLEAR_FAST((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
  spiFlashWrite(&buf, 1);
  spiFlashRead(&buf, 1);
  NRF_GPIO_PIN_SET_FAST((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
  return buf;
}

static void flashReset(){
  unsigned char buf[1];
  buf[0] = 0x66;
  spiFlashWriteCS(buf,1);
  buf[0] = 0x99;
  spiFlashWriteCS(buf,1);
  nrf_delay_us(50);
}

// Wake up the SPI Flash from deep power-down mode
static void flashWakeUp() {
  unsigned char buf = 0xAB;  // SPI Flash release from deep power-down
  spiFlashWriteCS(&buf,1);
  nrf_delay_us(50); // Wait at least 20us for Flash IC to wake up from deep power-down
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
#ifdef SPIFLASH_SLEEP_CMD
  // Release from deep power-down - might need a couple of attempts...
  flashReset();
  flashWakeUp();
  flashWakeUp();
  flashWakeUp();
  flashWakeUp();
#endif  
  // disable block protect 0/1/2
  unsigned char buf[2];
  int tries = 3;
  // disable lock bits on SPI flash
  do {
    // wait for write enable
    int timeout = 1000;
    while (timeout-- && !(spiFlashStatus()&2)) {
      buf[0] = 6; // write enable
      spiFlashWriteCS(buf,1);
      jshDelayMicroseconds(10);
    }
    jshDelayMicroseconds(10);
    buf[0] = 1; // write status register, disable BP0/1/2
    buf[1] = 0;
    spiFlashWriteCS(buf,2);
    jshDelayMicroseconds(10);
    // keep trying in case it didn't work first time
  } while (tries-- && (spiFlashStatus()&28)/*check BP0/1/2*/);
  // give flash time to boot? Some devices need this it seems.
  jshDelayMicroseconds(100000);
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

bool flashEqual(FlashHeader header, uint32_t addr) {
  unsigned char buf[256];
  unsigned int size = header.size;
  int inaddr = addr + sizeof(FlashHeader);
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

#if defined(LCD_CONTROLLER_GC9A01)
// LCD output for generic SPI LCDs
__attribute__( ( long_call, section(".data") ) ) void xlcd_wr(int data) {
  for (int bit=7;bit>=0;bit--) {
    NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_SCK, 0 );
    NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_MOSI, ((data>>bit)&1) );
    NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_SCK, 1 );
  }
}
#endif

__attribute__( ( long_call, section(".data") ) ) void xlcd_rect(int x1,int y1, int x2, int y2, bool white) {
#if defined(LCD_CONTROLLER_GC9A01)
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
    xlcd_wr(white ? 0xFF : 0);
  NRF_GPIO_PIN_WRITE_FAST(LCD_SPI_CS,1);
#endif
}

__attribute__( ( long_call, section(".data") ) ) void flashDoUpdate(FlashHeader header, uint32_t headerAddr) {
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
    xlcd_rect(60,182,60+percent,188,true);
    NRF_WDT->RR[0] = 0x6E524635; // kick watchdog
  }
  // Write
  size = header.size;
  int inaddr = headerAddr + sizeof(FlashHeader);
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
    xlcd_rect(60,192,60+percent,198,true);
    NRF_WDT->RR[0] = 0x6E524635; // kick watchdog
  }
  // clear progress bar
  xlcd_rect(60,180,180,200,false);
  // re-read all data just to try and clear any caches
  size = header.size;
  outaddr = header.address;
  volatile int v;
  while (size--) {
    v = *(char*)outaddr;
    outaddr++;
  }
  // done!
  while (true) NVIC_SystemReset(); // reset!
}

/// Return the size in bytes of a file based on the header
static uint32_t jsfGetFileSize(JsfFileHeader *header) {
  return (uint32_t)(header->size & 0x00FFFFFF);
}

/// Aligns a block, pushing it along in memory until it reaches the required alignment
static uint32_t jsfAlignAddress(uint32_t addr) {
  return (addr + (JSF_ALIGNMENT-1)) & (uint32_t)~(JSF_ALIGNMENT-1);
}

/** Load a file header from flash, return true if it is valid.
 * If readFullName==false, only the first 4 bytes of the name are loaded */
static bool jsfGetFileHeader(uint32_t addr, JsfFileHeader *header) {
  spiFlashReadAddr((unsigned char*)header, addr, sizeof(JsfFileHeader));
  return (header->size != 0xFFFFFFFF) &&
    (addr+(uint32_t)sizeof(JsfFileHeader)+jsfGetFileSize(header) <= JSF_END_ADDRESS);
}

// Get the address of the page after the current one, or 0. THE NEXT PAGE MAY HAVE A PREVIOUS PAGE'S DATA SPANNING OVER IT
static uint32_t jsfGetAddressOfNextPage(uint32_t addr) {
  addr = (uint32_t)(addr & ~(SPIFLASH_PAGESIZE-1)) + SPIFLASH_PAGESIZE;
  if (addr>=JSF_END_ADDRESS) return 0; // no pages in range
  return addr;
}

static bool jsfGetNextFileHeader(uint32_t *addr, JsfFileHeader *header) {
  uint32_t oldAddr = *addr;
  *addr = 0;
  // Work out roughly where the start is
  uint32_t newAddr = oldAddr + jsfGetFileSize(header) + (uint32_t)sizeof(JsfFileHeader);
  // pad out to flash write boundaries
  newAddr = jsfAlignAddress(newAddr);
  // sanity check for bad data
  if (newAddr<oldAddr) return 0; // corrupt!
  if (newAddr+sizeof(JsfFileHeader)>JSF_END_ADDRESS) return 0; // not enough space
  *addr = newAddr;
  bool valid = jsfGetFileHeader(newAddr, header);
  if (!valid) {
    // there wasn't another header in this page - check the next page
    newAddr = jsfGetAddressOfNextPage(newAddr);
    *addr = newAddr;
    if (!newAddr) return false; // no valid address
    valid = jsfGetFileHeader(newAddr, header);
    // we can't have a blank page and then a header, so stop our search
  }
  return valid;
}

void flashCheckFile(uint32_t fileAddr) {
  FlashHeader header;
  spiFlashReadAddr((unsigned char *)&header, fileAddr, sizeof(FlashHeader));
  if (header.address==0xFFFFFFFF || header.size==0) {
    // Not set - silently exit
    return;
  }
  lcd_println("FLASH HEADER");
  lcd_print_hex(header.address); lcd_println(" ADDR");
  lcd_print_hex(header.size); lcd_println(" SIZE");
  lcd_print_hex(header.CRC); lcd_println(" CRC");
  lcd_print_hex(header.version); lcd_println(" VERSION");
  if (header.address==0xf7000) return; // NO BOOTLOADER - FOR TESTING
  // Calculate CRC
  lcd_println("CRC TEST...");
  unsigned char buf[256];
  int size = header.size;
  int inaddr = fileAddr + sizeof(FlashHeader);
  uint32_t crc = 0;
  while (size>0) {
    unsigned int l = size;
    if (l>sizeof(buf)) l=sizeof(buf);
    spiFlashReadAddr(buf, inaddr, l);
    crc = crc32_compute(buf, l, &crc);
    inaddr += l;
    size -= l;
  }
  bool isEqual = false;
  if (crc != header.CRC) {
    // CRC is wrong - exits
    lcd_println("CRC MISMATCH");
    lcd_println("NOT FLASHING");
    lcd_print_hex(crc); lcd_println("");lcd_println("");
    for (volatile int i=0;i<5000000;i++) NRF_WDT->RR[0] = 0x6E524635; // delay
    // don't flash if the CRC doesn't match
    return;
  } else {
    // All ok - check we haven't already flashed this!
    lcd_println("CRC OK");
    lcd_println("TESTING...");
    isEqual = flashEqual(header, fileAddr);
  }
  lcd_println("REMOVE HEADER.");
  // Now erase the 'name' from the file header
  // which basically erases the file, so we don't
  // get into a boot loop
  unsigned char b[32];
  b[0] = 0x06; // WREN
  spiFlashWriteCS(b,1);
  uint32_t headerNameAddr = fileAddr-28; // name=28 chars
  for (volatile int i=0;i<1000;i++);
  b[0] = 0x02; // Write
  b[1] = headerNameAddr>>16;
  b[2] = headerNameAddr>>8;
  b[3] = headerNameAddr;
  memset(&b[4], 0, 28);
  spiFlashWriteCS(b,4+28); // write command plus 28 bytes of zeros
  // Check if flash busy
  while (spiFlashStatus()&1); // while 'Write in Progress'...
  // assume it's erased...

  if (!isEqual) {
    lcd_println("FIRMWARE DIFFERENT.");
    lcd_println("FLASHING...");

    xlcd_rect(60,180,180,180,true);
    xlcd_rect(60,190,180,190,true);
    xlcd_rect(60,200,180,200,true);

    flashDoUpdate(header, fileAddr);

#if 0
    isEqual = flashEqual(header, addr);
    if (isEqual) lcd_println("EQUAL");
    else lcd_println("NOT EQUAL");

    for (volatile int i=0;i<5000000;i++) NRF_WDT->RR[0] = 0x6E524635; // delay
    while (true) NVIC_SystemReset();
#endif
  } else {
    lcd_println("BINARY MATCHES.");
  }
}


void flashCheckAndRun() {
  lcd_println("CHECK STORAGE");
  spiFlashInit();

  uint32_t addr = JSF_START_ADDRESS;
  JsfFileHeader header;
  //lcd_print_hex(addr); lcd_println(" ADDR");
  int tries = 50000;
  if (jsfGetFileHeader(addr, &header)) do {
    if (tries-- < 0) {
      lcd_println("TOO MANY FILES");
      return;
    }
    char *n = &header.name[0];
    /*lcd_print_hex(addr);
    if (n[0]) lcd_println(n); else lcd_println("-DELETED-");*/
    if (n[0]=='.' && n[1]=='f' && n[2]=='i' && n[3]=='r' && n[4]=='m' && n[5]=='w' && n[6]=='a' && n[7]=='r' && n[8]=='e' && n[9]==0) {
      lcd_println("FOUND FIRMWARE");
      flashCheckFile(addr + sizeof(JsfFileHeader)/*, jsfGetFileSize(&header)*/);
      return;
    }
  } while (jsfGetNextFileHeader(&addr, &header));
  lcd_println("NO NEW FW");
}

// Put the SPI Flash into deep power-down mode
void flashPowerDown() {
  spiFlashInit(); // 
  unsigned char buf = 0xB9;  // SPI Flash deep power-down
  spiFlashWriteCS(&buf,1);
  nrf_delay_us(2); // Wait at least 1us for Flash IC to enter deep power-down
}


#endif
