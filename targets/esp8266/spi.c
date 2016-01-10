/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (c) 2015 David Ogilvy (MetalPhreak)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Contains ESP8266 board specific functions to drive the hardware SPI interface.
 * ----------------------------------------------------------------------------
 */

/**
 * The Github project from which this source file was adapted from can be found
 * at:  https://github.com/MetalPhreak/ESP8266_SPI_Driver
 */


#include "spi.h"
#include "user_interface.h"
#include "espmissingincludes.h"

//#define SPI_CLK_USE_DIV 0
//#define SPI_CLK_80MHZ_NODIV 1

#define SPI_BYTE_ORDER_HIGH_TO_LOW 1
#define SPI_BYTE_ORDER_LOW_TO_HIGH 0

//#ifndef CPU_CLK_FREQ //Should already be defined in eagle_soc.h
//#define CPU_CLK_FREQ 80*1000000
//#endif

// Define some default SPI clock settings
//#define SPI_CLK_PREDIV 10
//#define SPI_CLK_CNTDIV 2
//#define SPI_CLK_FREQ CPU_CLK_FREQ/(SPI_CLK_PREDIV*SPI_CLK_CNTDIV) // 80 / 20 = 4 MHz

// forward declarations
static void spi_init_gpio(uint8 spi_no, uint8 sysclk_as_spiclk);
static void spi_clock(uint8 spi_no, uint16 prediv, uint8 cntdiv);
static void spi_tx_byte_order(uint8 spi_no, uint8 byte_order);
static void spi_rx_byte_order(uint8 spi_no, uint8 byte_order);

// spi_init -- Wrapper to setup HSPI/SPI GPIO pins and default SPI clock
// Parameters: spi_no - SPI (0) or HSPI (1)
void
spi_init(uint8 spi_no, uint32 baud_rate)
{
  if(spi_no > 1) return; //Only SPI and HSPI are valid spi modules.

  uint32_t freq = system_get_cpu_freq(); // returns 80 or 160

  spi_init_gpio(spi_no, 0);
  spi_clock(spi_no, 20000000/baud_rate, 4); // the base clock is always 80Mhz
  spi_tx_byte_order(spi_no, SPI_BYTE_ORDER_HIGH_TO_LOW);
  spi_rx_byte_order(spi_no, SPI_BYTE_ORDER_HIGH_TO_LOW);

  SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_DOUTDIN|SPI_CS_SETUP|SPI_CS_HOLD);
  CLEAR_PERI_REG_MASK(SPI_USER(spi_no), SPI_FLASH_MODE);
}

// spi_init_gpio -- Initialises the GPIO pins for use as SPI pins.
// Parameters: spi_no - SPI (0) or HSPI (1)
//             sysclk_as_spiclk - SPI_CLK_80MHZ_NODIV (1) if using 80MHz sysclock for SPI clock.
//                   SPI_CLK_USE_DIV (0) if using divider to get lower SPI clock speed.
static void
spi_init_gpio(uint8 spi_no, uint8 sysclk_as_spiclk)
{

//  if(spi_no > 1) return; //Not required. Valid spi_no is checked with if/elif below.

  uint32 clock_div_flag = 0;
  if(sysclk_as_spiclk){
    clock_div_flag = 0x0001;
  }

  if(spi_no==SPI){
    WRITE_PERI_REG(PERIPHS_IO_MUX, 0x005|(clock_div_flag<<8)); //Set bit 8 if 80MHz sysclock required
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_CLK_U, 1);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_CMD_U, 1);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA0_U, 1);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA1_U, 1);
  }else if(spi_no==HSPI){
    WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105|(clock_div_flag<<9)); //Set bit 9 if 80MHz sysclock required
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2); //GPIO12 is HSPI MISO pin (Master Data In)
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2); //GPIO13 is HSPI MOSI pin (Master Data Out)
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2); //GPIO14 is HSPI CLK pin (Clock)
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, 2); //GPIO15 is HSPI CS pin (Chip Select / Slave Select)
  }

}

// spi_clock -- sets up the control registers for the SPI clock
// Parameters:
//   spi_no - SPI (0) or HSPI (1)
//   prediv - predivider value (actual division value)
//   cntdiv - postdivider value (actual division value)
//   Set either divider to 0 to disable all division (80MHz sysclock)
static void
spi_clock(uint8 spi_no, uint16 prediv, uint8 cntdiv)
{
  if(spi_no > 1) return;

  if ((prediv==0)||(cntdiv==0)) {
    WRITE_PERI_REG(SPI_CLOCK(spi_no), SPI_CLK_EQU_SYSCLK);
  } else {
    //os_printf("SPI clk: %d %d -> %d, %d %d %d\n", prediv, cntdiv, prediv-1, cntdiv<<1, cntdiv, cntdiv<<1);
    WRITE_PERI_REG(SPI_CLOCK(spi_no),
          (((prediv-1)&SPI_CLKDIV_PRE)<<SPI_CLKDIV_PRE_S)|
          (((cntdiv-1)&SPI_CLKCNT_N)<<SPI_CLKCNT_N_S)|
          ((((cntdiv+1)/2-1)&SPI_CLKCNT_H)<<SPI_CLKCNT_H_S)|
          (((cntdiv-1)&SPI_CLKCNT_L)<<SPI_CLKCNT_L_S));
  }
}

// spi_tx_byte_order -- Setup the byte order for shifting data out of buffer
// Parameters:
//   spi_no - SPI (0) or HSPI (1)
//   byte_order - SPI_BYTE_ORDER_HIGH_TO_LOW (1)
//                  Data is sent out starting with Bit31 and down to Bit0
//                SPI_BYTE_ORDER_LOW_TO_HIGH (0)
//                  Data is sent out starting with the lowest BYTE, from
//                  MSB to LSB, followed by the second lowest BYTE, from
//                  MSB to LSB, followed by the second highest BYTE, from
//                  MSB to LSB, followed by the highest BYTE, from MSB to LSB
//                  0xABCDEFGH would be sent as 0xGHEFCDAB
static void
spi_tx_byte_order(uint8 spi_no, uint8 byte_order)
{
  if(spi_no > 1) return;

  if (byte_order) {
    SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_WR_BYTE_ORDER);
  } else {
    CLEAR_PERI_REG_MASK(SPI_USER(spi_no), SPI_WR_BYTE_ORDER);
  }
}

// spi_rx_byte_order -- Setup the byte order for shifting data into buffer
// Parameters:
//   spi_no - SPI (0) or HSPI (1)
//   byte_order - SPI_BYTE_ORDER_HIGH_TO_LOW (1)
//                  Data is read in starting with Bit31 and down to Bit0
//                SPI_BYTE_ORDER_LOW_TO_HIGH (0)
//                  Data is read in starting with the lowest BYTE, from
//                  MSB to LSB, followed by the second lowest BYTE, from
//                  MSB to LSB, followed by the second highest BYTE, from
//                  MSB to LSB, followed by the highest BYTE, from MSB to LSB
//                  0xABCDEFGH would be read as 0xGHEFCDAB
static void
spi_rx_byte_order(uint8 spi_no, uint8 byte_order)
{
  if(spi_no > 1) return;

  if (byte_order){
    SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_RD_BYTE_ORDER);
  } else {
    CLEAR_PERI_REG_MASK(SPI_USER(spi_no), SPI_RD_BYTE_ORDER);
  }
}

// spi_transaction -- SPI transaction function
//
// Parameters:
//   spi_no - SPI (0) or HSPI (1)
//   bits - actual number of bits to transmit
//   dout_data - output data
// Returns:
//   read data - uint32 containing read in data
// Note: all data is assumed to be stored in the lower bits of
//       the data variables (for anything <32 bits).
uint32
spi_transaction(uint8 spi_no, uint32 bits, uint32 dout_data)
{
  if(spi_no > 1) return 0;  //Check for a valid SPI

  // TODO: code for custom Chip Select as GPIO PIN here

  while(spi_busy(spi_no)); //wait for SPI to be ready

  // Enable SPI Functions
  CLEAR_PERI_REG_MASK(SPI_USER(spi_no),
      SPI_FLASH_MODE|SPI_USR_MOSI|SPI_USR_MISO|SPI_USR_COMMAND|SPI_USR_ADDR|SPI_USR_DUMMY);
  SET_PERI_REG_MASK(SPI_USER(spi_no),
      SPI_USR_MOSI|SPI_DOUTDIN|SPI_CK_I_EDGE); // make sure we get out & in full-duplex

  // Setup Bitlengths
  WRITE_PERI_REG(SPI_USER1(spi_no),
      ((bits-1)&SPI_USR_MOSI_BITLEN)<<SPI_USR_MOSI_BITLEN_S |  // Number of bits to Send
      ((bits-1)&SPI_USR_MISO_BITLEN)<<SPI_USR_MISO_BITLEN_S);  // Number of bits to receive

  //copy data to W0
  if(READ_PERI_REG(SPI_USER(spi_no))&SPI_WR_BYTE_ORDER) {
    WRITE_PERI_REG(SPI_W0(spi_no), dout_data<<(32-bits));
  } else {
    uint8 dout_extra_bits = bits%8;

    if (dout_extra_bits){
      // if your data isn't a byte multiple (8/16/24/32 bits) and you don't have
      // SPI_WR_BYTE_ORDER set, you need this to move the non-8bit remainder to the MSBs
      // not sure if there's even a use case for this, but it's here if you need it...
      // for example, 0xDA4 12 bits without SPI_WR_BYTE_ORDER would usually be output as if it were 0x0DA4,
      // of which 0xA4, and then 0x0 would be shifted out (first 8 bits of low byte, then
      // 4 MSB bits of high byte - ie reverse byte order).
      // The code below shifts it out as 0xA4 followed by 0xD as you might require.
      WRITE_PERI_REG(SPI_W0(spi_no),
          (0xFFFFFFFF<<(bits - dout_extra_bits)&dout_data)<<(8-dout_extra_bits) |
          (0xFFFFFFFF>>(32-(bits - dout_extra_bits)))&dout_data);
    } else {
      WRITE_PERI_REG(SPI_W0(spi_no), dout_data);
    }
  }

  // Begin SPI Transaction
  SET_PERI_REG_MASK(SPI_CMD(spi_no), SPI_USR);

  // Return DIN data
  while (spi_busy(spi_no)) ;  //wait for SPI transaction to complete

  if (READ_PERI_REG(SPI_USER(spi_no))&SPI_RD_BYTE_ORDER) {
    return READ_PERI_REG(SPI_W0(spi_no)) >> (32-bits); //Assuming data in is written to MSB. TBC
  } else {
    return READ_PERI_REG(SPI_W0(spi_no)); //Read in the same way as DOUT is sent.
                               // Note existing contents of SPI_W0 remain unless overwritten!
  }
}
