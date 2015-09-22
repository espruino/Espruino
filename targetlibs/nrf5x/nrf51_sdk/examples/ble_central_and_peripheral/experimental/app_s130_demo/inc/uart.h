/* Copyright (c) 2014, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   * Neither the name of Nordic Semiconductor ASA nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>
#include <stdbool.h>

/** Some definitions needed for the ported console functions */
#define UART_MODE_GENERIC_POLLING 0

/* Newline character sequences */
#define UART_NEWLINE_CRLF          "\r\n"
#define UART_NEWLINE_CR            "\r"
#define UART_NEWLINE_LF            "\n"
#define UART_NEWLINE_CRLF_NUMERIC  "\x0D\x0A"  /* Hardcoded ASCII values for CRLF */
#define UART_NEWLINE_CR_NUMERIC    "\x0D"      /* It is possible, though unlikely, that */
#define UART_NEWLINE_LF_NUMERIC    "\x0A"      /* \r and \n do not equal these values. */
/* Default newline style */
#define UART_NEWLINE_DEFAULT       UART_NEWLINE_CRLF_NUMERIC

/* Newline style for input */
#ifndef UART_NEWLINE_INPUT
  /* UART_NEWLINE_INPUT was not defined in console_config.h */
  #define UART_NEWLINE_INPUT      UART_NEWLINE_DEFAULT
#endif
  
/* Newline style for output */
#ifndef UART_NEWLINE_OUTPUT
  /* UART_NEWLINE_OUTPUT was not defined in console_config.h */
  #define UART_NEWLINE_OUTPUT     UART_NEWLINE_DEFAULT
#endif


/** Available Baud rates for UART. */
typedef enum
{
    UART_BAUD_2K4 = 0,        ///< 2400 baud
    UART_BAUD_4K8,            ///< 4800 baud
    UART_BAUD_9K6,            ///< 9600 baud
    UART_BAUD_14K4,           ///< 14.4 kbaud
    UART_BAUD_19K2,           ///< 19.2 kbaud
    UART_BAUD_28K8,           ///< 28.8 kbaud
    UART_BAUD_38K4,           ///< 38.4 kbaud
    UART_BAUD_57K6,           ///< 57.6 kbaud
    UART_BAUD_76K8,           ///< 76.8 kbaud
    UART_BAUD_115K2,          ///< 115.2 kbaud
    UART_BAUD_230K4,          ///< 230.4 kbaud
    UART_BAUD_250K0,          ///< 250.0 kbaud
    UART_BAUD_500K0,          ///< 500.0 kbaud
    UART_BAUD_1M0,            ///< 1 mbaud
    UART_BAUD_TABLE_MAX_SIZE  ///< Used to specify the size of the baudrate table.
} uart_baudrate_t;

/** @brief The baudrate devisors array, calculated for standard baudrates.
    Number of elements defined by ::HAL_UART_BAUD_TABLE_MAX_SIZE*/
#define UART_BAUDRATE_DEVISORS_ARRAY    { \
    0x0009D000, 0x0013B000, 0x00275000, 0x003BA000, 0x004EA000, 0x0075F000, 0x009D4000, \
    0x00EBE000, 0x013A9000, 0x01D7D000, 0x03AFB000, 0x03FFF000, 0x075F6000, 0x10000000  }

/** @brief This function initializes the UART.
 *
 * @param baud_rate - the baud rate to be used, ::uart_baudrate_t.
 *
 */
void uart_init(uart_baudrate_t const baud_rate);

/** @brief Find number of characters in the UART receive buffer
 *
 * This function returns the number of characters available for reading
 * in the UART receive buffer.
 * 
 * @return Number of characters available
 */
bool uart_chars_available(void);

/** Function to write a character to the UART transmit buffer.
 * @param ch Character to write
 */
void uart_putchar(uint8_t ch);

/** Function to read a character from the UART receive buffer.
 * @return Character read
 */
uint8_t uart_getchar(void);

/** Function to write and array of byts to UART.
 *
 * @param buf - buffer to be printed on the UART.
 * @param len - Number of bytes in buffer.
 */
void uart_write_buf(uint8_t const *buf, uint32_t len);

#endif // __UART_H__
