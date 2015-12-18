/**************************************************************************//**
 * @file
 * @brief NAND Flash example for EFM32GG_STK3700 development kit
 * @version 4.2.1
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_common.h"
#include "bsp.h"
#include "retargetserial.h"
#include "bsp_trace.h"

#include "nandflash.h"

/**************************************************************************//**
 *
 * This example demonstrates use of NAND Flash on STK3700.
 *
 * Connect a terminal to the serialport (115200-N-8-1) on EXP port pins
 * 2 and 4.
 * Operations on the flash are initiated by issuing commands on the terminal.
 * Command "h" will print a help screen on the terminal.
 *
 *****************************************************************************/

#define PAGENUM_2_ADDR(x)  ( ( (x) * NANDFLASH_DeviceInfo()->pageSize ) + \
                               NANDFLASH_DeviceInfo()->baseAddress )
#define BLOCKNUM_2_ADDR(x) ( ( (x) * NANDFLASH_DeviceInfo()->blockSize ) + \
                               NANDFLASH_DeviceInfo()->baseAddress )
#define ADDR_2_PAGENUM(x)  ( ( (x) - NANDFLASH_DeviceInfo()->baseAddress ) / \
                             NANDFLASH_DeviceInfo()->pageSize)
#define ADDR_2_BLOCKNUM(x) ( ( (x) - NANDFLASH_DeviceInfo()->baseAddress ) / \
                             NANDFLASH_DeviceInfo()->blockSize)

#define DWT_CYCCNT  *(volatile uint32_t*)0xE0001004
#define DWT_CTRL    *(volatile uint32_t*)0xE0001000

/** Command line */
#define CMDBUFSIZE    80
#define MAXARGC       5

/** RS232 Input buffer */
static char cmdBuffer[ CMDBUFSIZE + 1 ];
static int  argc;
static char *argv[ MAXARGC ];

/* NOTE: We assume that page size is 512 !! */
#define BUF_SIZ 512
EFM32_ALIGN(4)
static uint8_t buffer[ 2 ][ BUF_SIZ ] __attribute__ ((aligned(4)));

static bool blankCheckPage( uint32_t addr, uint8_t *buffer );
static void dump16( uint32_t addr, uint8_t *data );
static void dumpPage( uint32_t addr, uint8_t *data );
static void getCommand( void );
static void printHelp( void );
static void splitCommandLine( void );

/**************************************************************************//**
 * @brief main - the entrypoint after reset.
 *****************************************************************************/
int main(void)
{
  uint32_t time;

  /* Chip errata */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Select 48MHz clock. */
  CMU_ClockSelectSet( cmuClock_HF, cmuSelect_HFXO );

  /* Setup EBI for NAND Flash. */
  BSP_EbiInit();

  /* Enable DWT */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  /* Make sure CYCCNT is running */
  DWT_CTRL |= 1;

  /* Initialize USART and map LF to CRLF */
  RETARGET_SerialInit();
  RETARGET_SerialCrLf(1);

  printf("\nEFM32GG_STK3700 NAND Flash example\n");
  printHelp();

  /* Initialize nand flash module, use DMA channel 5. */
  NANDFLASH_Init( 5 );

  while (1)
  {
    getCommand();

    /* Get misc. NAND flash info */
    if ( !strcmp( argv[0], "fi" ) )
    {
      printf( " NAND flash device information:\n" );
      printf( "\n  Manufacturer Code :  0x%02X", NANDFLASH_DeviceInfo()->manufacturerCode );
      printf( "\n  Device Code       :  0x%02X", NANDFLASH_DeviceInfo()->deviceCode );
      printf( "\n  Device size       :  %ld (%ldMB)", NANDFLASH_DeviceInfo()->deviceSize,
                                                      NANDFLASH_DeviceInfo()->deviceSize/1024/1024 );
      printf( "\n  Page size         :  %ld", NANDFLASH_DeviceInfo()->pageSize );
      printf( "\n  Spare size        :  %ld", NANDFLASH_DeviceInfo()->spareSize );
      printf( "\n  Block size        :  %ld", NANDFLASH_DeviceInfo()->blockSize );
      putchar( '\n' );
    }

    /* Read a page */
    else if ( !strcmp( argv[0], "rp" ) )
    {
      int status;
      uint32_t pageNum, addr;

      pageNum = strtoul( argv[1], NULL, 0 );
      addr = PAGENUM_2_ADDR( pageNum );

      if ( !NANDFLASH_AddressValid( addr ) )
      {
        printf( " Read page content, page %ld is not a valid page\n", pageNum );
      }
      else
      {
        time = DWT_CYCCNT;
        status = NANDFLASH_ReadPage( addr, buffer[0] );
        time = DWT_CYCCNT - time;
        if ( status == NANDFLASH_STATUS_OK )
        {
          printf( " Read page %ld content, %ld cpu-cycles used\n", pageNum, time );
        }
        else
        {
          printf( " Read page error %d, %ld cpu-cycles used\n", status, time );
        }
        dumpPage( addr, buffer[0] );
      }
    }

    /* Blankcheck a page */
    else if ( !strcmp( argv[0], "bp" ) )
    {
      uint32_t pageNum, addr;

      pageNum = strtoul( argv[1], NULL, 0 );
      addr = PAGENUM_2_ADDR( pageNum );

      if ( !NANDFLASH_AddressValid( addr ) )
      {
        printf( " Blankcheck page, page %ld is not a valid page\n", pageNum );
      }
      else
      {
        printf( " Blankchecking page %ld\n", pageNum );
        if ( blankCheckPage( addr, buffer[0] ) )
        {
          printf( " Page %ld is blank\n", pageNum );
        }
      }
    }

    /* Blankcheck entire device */
    else if ( !strcmp( argv[0], "bd" ) )
    {
      uint32_t addr;
      int i, pageSize, pageCount;

      pageSize  = NANDFLASH_DeviceInfo()->pageSize;
      pageCount = NANDFLASH_DeviceInfo()->deviceSize / pageSize;
      addr      = NANDFLASH_DeviceInfo()->baseAddress;

      printf( " Blankchecking entire device\n" );
      for ( i=0; i<pageCount; i++, addr+=pageSize )
      {
        if ( !blankCheckPage( addr, buffer[0] ) )
        {
          break;
        }
      }

      if ( i == pageCount )
      {
        printf( " Device is blank\n" );
      }
    }

    /* Check bad-block info */
    else if ( !strcmp( argv[0], "bb" ) )
    {
      uint32_t addr;
      int i, blockSize, blockCount, badBlockCount = 0;

      blockCount = NANDFLASH_DeviceInfo()->deviceSize / NANDFLASH_DeviceInfo()->blockSize;
      blockSize  = NANDFLASH_DeviceInfo()->blockSize;
      addr       = NANDFLASH_DeviceInfo()->baseAddress;

      for ( i=0; i<blockCount; i++, addr+=blockSize )
      {
        NANDFLASH_ReadSpare( addr, buffer[0] );
        /* Manufacturer puts bad-block info in byte 6 of the spare area */
        /* of the first page in each block.                             */
        if ( buffer[0][NAND_SPARE_BADBLOCK_POS] != 0xFF )
        {
          printf( " ---> Bad-block at address 0x%08lX (block %ld) <---\n",
                  addr, ADDR_2_BLOCKNUM(addr) );
          badBlockCount++;
        }
      }

      if ( badBlockCount == 0 )
      {
        printf( " Device has no bad-blocks\n" );
      }
    }

    /* Write a page */
    else if ( !strcmp( argv[0], "wp" ) )
    {
      int i, status;
      uint32_t pageNum, addr;

      pageNum = strtoul( argv[1], NULL, 0 );
      addr = PAGENUM_2_ADDR( pageNum );

      if ( !NANDFLASH_AddressValid( addr ) )
      {
        printf( " Write page, page %ld is not a valid page\n", pageNum );
      }
      else
      {
        printf( " Write page %ld, ", pageNum );

        for ( i=0; i<BUF_SIZ; i++ )
        {
          buffer[0][i] = i;
        }

        time = DWT_CYCCNT;
        status = NANDFLASH_WritePage( addr, buffer[0] );
        time = DWT_CYCCNT - time;
        printf( "ecc : 0x%08lX\n", NANDFLASH_DeviceInfo()->ecc );

        if ( status == NANDFLASH_STATUS_OK )
        {
          printf( " Page written OK, %ld cpu-cycles used\n", time );
        }
        else if ( status == NANDFLASH_WRITE_ERROR )
        {
          printf( " Page write failure, bad-block\n" );
        }
        else
        {
          printf( " Page write error %d\n", status );
        }
      }
    }

    /* Erase a block */
    else if ( !strcmp( argv[0], "eb" ) )
    {
      int status;
      uint32_t blockNum, addr;

      blockNum = strtoul( argv[1], NULL, 0 );
      addr = BLOCKNUM_2_ADDR( blockNum );

      if ( !NANDFLASH_AddressValid( addr ) )
      {
        printf( " Erase block, block %ld is not a valid block\n", blockNum );
      }
      else
      {
        printf( " Erase block %ld, ", blockNum );

        status = NANDFLASH_EraseBlock( addr );

        if ( status == NANDFLASH_STATUS_OK )
        {
          printf( " Block erased OK\n" );
        }
        else if ( status == NANDFLASH_WRITE_ERROR )
        {
          printf( " Block erase failure, bad-block\n" );
        }
        else
        {
          printf( " Block erase error %d\n", status );
        }
      }
    }

    /* Check ECC algorithm */
    else if ( !strcmp( argv[0], "ecc" ) )
    {
      int i, status;
      uint32_t addr0, addr1, ecc0, ecc1, pageNum;

      for ( i=0; i<BUF_SIZ; i++ )
      {
        buffer[0][i] = i;
        buffer[1][i] = i;
      }

      pageNum = strtoul( argv[1], NULL, 0 );
      addr0   = PAGENUM_2_ADDR( pageNum );
      addr1   = PAGENUM_2_ADDR( pageNum + 1 );

      if ( !NANDFLASH_AddressValid( addr0 ) ||
           !NANDFLASH_AddressValid( addr1 )    )
      {
        printf( " Check ECC algorithm, page %ld or %ld is not a valid page\n",
                pageNum, pageNum+1 );
      }
      else
      {
        printf( " Checking ECC algorithm\n" );
        status = NANDFLASH_WritePage( addr0, buffer[0] );
        if ( status != NANDFLASH_STATUS_OK )
        {
          printf( " Write in page <n> failed\n" );
        }
        ecc0 = NANDFLASH_DeviceInfo()->ecc; /* Get ECC generated during write*/

        /* Patch one bit in the second buffer. */
        buffer[1][147] ^= 4;

        status = NANDFLASH_WritePage( addr1, buffer[1] );
        if ( status != NANDFLASH_STATUS_OK )
        {
          printf( " Write in page <n+1> failed\n" );
        }
        ecc1 = NANDFLASH_DeviceInfo()->ecc; /* Get ECC generated during write*/

        /* Try to correct the second buffer using ECC from first buffer */
        NANDFLASH_EccCorrect( ecc0, ecc1, buffer[1] );
        if ( memcmp( buffer[0], buffer[1], BUF_SIZ ) == 0 )
        {
          printf( " ECC correction succeeded\n" );
        }
        else
        {
          printf( " ECC correction failed\n" );
        }

        /* Byte 147 (addr 0x93) of page n+1 is now 0x97.                    */
        /* Single step next function call, stop when ECC is read, set the   */
        /* read ECC to ecc10 (0) and verify that 0x97 is corrected to 0x93! */
        NANDFLASH_ReadPage( addr1, buffer[1] );
      }
    }

    /* Copy a page */
    else if ( !strcmp( argv[0], "cp" ) )
    {
      int status;
      uint32_t dstPageNum, srcPageNum, dstAddr, srcAddr;

      dstPageNum = strtoul( argv[2], NULL, 0 );
      srcPageNum = strtoul( argv[1], NULL, 0 );
      dstAddr    = PAGENUM_2_ADDR( dstPageNum );
      srcAddr    = PAGENUM_2_ADDR( srcPageNum );

      if ( !NANDFLASH_AddressValid( dstAddr ) ||
           !NANDFLASH_AddressValid( srcAddr )    )
      {
        printf( " Copy page, page %ld or %ld is not a valid page\n",
                srcPageNum, dstPageNum );
      }
      else
      {
        printf( " Copying page %ld to page %ld\n", srcPageNum, dstPageNum );

        status = NANDFLASH_CopyPage( dstAddr, srcAddr );

        if ( status == NANDFLASH_STATUS_OK )
        {
          printf( " Page copied OK\n" );
        }
        else if ( status == NANDFLASH_WRITE_ERROR )
        {
          printf( " Page copy failure, bad-block\n" );
        }
        else
        {
          printf( " Page copy error %d\n", status );
        }
      }
    }

    /* Mark block as bad */
    else if ( !strcmp( argv[0], "mb" ) )
    {
      uint32_t blockNum, addr;

      blockNum = strtoul( argv[1], NULL, 0 );
      addr = BLOCKNUM_2_ADDR( blockNum );

      if ( !NANDFLASH_AddressValid( addr ) )
      {
        printf( " Mark bad block, %ld is not a valid block\n", blockNum );
      }
      else
      {
        printf( " Marked block %ld as bad\n", blockNum );
        NANDFLASH_MarkBadBlock( addr );
      }
    }

    /* Display help */
    else if ( !strcmp( argv[0], "h" ) )
    {
      printHelp();
    }
    else
    {
      printf( " Unknown command" );
    }
  }
}

/**************************************************************************//**
 * @brief Print a help screen.
 *****************************************************************************/
static void printHelp( void )
{
  printf(
    "Available commands:\n"
    "\n    fi         : Show NAND flash device information"
    "\n    h          : Show this help"
    "\n    rp <n>     : Read page <n>"
    "\n    bp <n>     : Blankcheck page <n>"
    "\n    bd         : Blankcheck entire device"
    "\n    bb         : Check bad-block info"
    "\n    mb <n>     : Mark block <n> as bad"
    "\n    wp <n>     : Write page <n>"
    "\n    eb <n>     : Erase block <n>"
    "\n    ecc <n>    : Check ECC algorithm, uses page <n> and <n+1>"
    "\n    cp <m> <n> : Copy page <m> to page <n>"
    "\n" );
}

/**************************************************************************//**
 * @brief Get a command from the terminal on the serialport.
 *****************************************************************************/
static void getCommand( void )
{
  int c;
  int index = 0;

  printf( "\n>" );
  while (1)
  {
    c = getchar();
    if (c > 0)
    {
      /* Output character - most terminals use CRLF */
      if (c == '\r')
      {
        cmdBuffer[index] = '\0';
        splitCommandLine();
        putchar( '\n' );
        return;
      }
      else if (c == '\b')
      {
        printf( "\b \b" );
        if ( index )
          index--;
      }
      else
      {
        /* Filter non-printable characters */
        if ((c < ' ') || (c > '~'))
          continue;

        /* Enter into buffer */
        cmdBuffer[index] = c;
        index++;
        if (index == CMDBUFSIZE)
        {
          cmdBuffer[index] = '\0';
          splitCommandLine();
          return;
        }
        /* Echo char */
        putchar(c);
      }
    }
  }
}

/**************************************************************************//**
 * @brief Split a command line into separate arguments.
 *****************************************************************************/
static void splitCommandLine( void )
{
  int i;
  char *result = strtok( cmdBuffer, " " );

  argc = 0;
  for( i=0; i < MAXARGC; i++ )
  {
    if ( result )
      argc++;

    argv[ i ] = result;
    result = strtok( NULL, " " );
  }
}

/**************************************************************************//**
 * @brief Dump page content on terminal.
 *
 * @param[in] addr
 *   The address to start at.
 * @param[in] data
 *   A buffer with page data.
 *****************************************************************************/
static void dumpPage( uint32_t addr, uint8_t *data )
{
  uint32_t i, j, lines;

  printf( "\nEcc     : 0x%08lX", NANDFLASH_DeviceInfo()->ecc );
  printf( "\nSpare   : " );

  for ( i=0; i<NANDFLASH_DeviceInfo()->spareSize; i++ )
  {
    printf( "%02X ", NANDFLASH_DeviceInfo()->spare[i] );
  }
  putchar( '\n' );

  lines = NANDFLASH_DeviceInfo()->pageSize / 16;
  for ( i=0     , j=0             ;
        i<lines                   ;
        i++     , j+=16, addr+=16   )
  {
    dump16( addr, &data[j] );
  }
  putchar( '\n' );
}

/**************************************************************************//**
 * @brief Print 16 bytes in both hex and ascii form on terminal.
 *
 * @param[in] addr
 *   The address to start at.
 * @param[in] data
 *   A buffer with 16 bytes to print.
 *****************************************************************************/
static void dump16( uint32_t addr, uint8_t *data )
{
  int i;

  printf( "\n%08lX: ", addr );

  /* Print data in hex format */
  for ( i=0; i<16; i++ )
  {
    printf( "%02X ", data[i] );
  }
  printf( "   " );
  /* Print data in ASCII format */
  for ( i=0; i<16; i++ )
  {
    if ( isprint( data[i] ) )
    {
      printf( "%c", data[i] );
    }
    else
    {
      printf( " " );
    }
  }
}

/**************************************************************************//**
 * @brief Blankcheck a page in NAND flash.
 *
 * @param[in] addr
 *   The first address in the nand flash to start blankchecking.
 *
 * @param[in] buffer
 *   Page buffer to use when reading from nand flash.
 *****************************************************************************/
static bool blankCheckPage( uint32_t addr, uint8_t *buffer )
{
  uint32_t i;

  NANDFLASH_ReadPage( addr, buffer );
  for ( i=0; i<NANDFLASH_DeviceInfo()->pageSize; i++ )
  {
    if ( buffer[i] != 0xFF )
    {
      printf( " ---> Blankcheck failure at address 0x%08lX (page %ld) <---\n",
              addr+i, ADDR_2_PAGENUM(addr) );
      return false;
    }
  }

  for ( i=0; i<NANDFLASH_DeviceInfo()->spareSize; i++ )
  {
    if ( NANDFLASH_DeviceInfo()->spare[i] != 0xFF )
    {
      printf( " ---> Blankcheck failure in spare area for page at address 0x%08lX (page %ld), spare byte number %ld <---\n",
              addr, ADDR_2_PAGENUM(addr), i );
      return false;
    }
  }
  return true;
}
