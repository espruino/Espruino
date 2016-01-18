/* Copyright (c) 2007 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */


/** \file
 *
 * \section console_description Description
 *
 * Higher-level functions for writing to, and reading from, the UART.
 *
 * Lower-level functions are provided by hal_uart.h and its implementations,
 * which this module uses to obtain hardware indpendency.
 *
 * Before the other functions of this module is used, the module must be initialized
 * by calling console_init().
 * Also, the interrupt used by the UART must be enabled.
 *
 * Functions for writing to the UART have a "put" infix, functions for reading 
 * from he UART a "get" infix.
 *
 * The newline problem is of interest.  See e.g.  
 * <URL: http://en.wikipedia.org/wiki/Newline >.
 * The newline style used by default by this module is ASCII CR and LF (x0D x0A).
 * The module may be configured to use other newline styles, including setting different
 * newline styles for input and output, see the section on configuration further below.
 * 
 * To print a newline only, call console_put_newline(), or console_put_line() with a 
 * properly casted empty string as argument:
 * - console_put_newline()
 * - console_put_line((uint8_t *)"")
 *
 * The newline solution is not completely general, see the comments to console_get_line().
 *
 * End-of-file and end-of-file representation may also be of concern.  This has not been 
 * dealt with in any way in this module.
 *
 * This module provides optional echo of input to output.  This may e.g. be used to enable
 * the other part to see what is sent from that side (if other part does not have local echo)
 * or to ensure that the UART communication is working.  See the configuration section for
 * info on how to turn on this feature.
 *
 * A note on input-to-output echo and newline: If echo is enabled and an input newline is read,
 * an output newline, which may be different (dependent upon module configuration), is echoed 
 * back.
 *
 * A note on terminology: In this file, what is meant by a "string" is a zero-terminated 
 * sequence (an array) of characters (or octets).  This as opposed to a sequence of characters
 * without such termination.
 *
 * String constants (like "this one") are (per the C standard) of type "array of character".
 * Therefore, Lint may complain if string constants are used as arguments to the functions 
 * in this module.  To avoid this, cast the string to uint8_t like this: 
 * console_put_string((uint8_t *)"this one").
 *
 * \section console_configuration Configuration
 * 
 * The module may be configured by the use of suitable defines.  To change the newline style
 * used, define CONSOLE_NEWLINE_INPUT and CONSOLE_NEWLINE_OUTPUT to suitable values.  (See 
 * this file (console.h) for possible values.)  To enable echoing of input to output, define
 * CONSOLE_ENABLE_ECHO.
 *
 * \section console_note Note
 * 
 * The console functions that access the UART driver will enable the rewired
 * interrupts for the driver.  This ensures that the driver will not stall and
 * lock the program.  The interrupts will be restored to their former value
 * before returning to the caller in each instance that this is used.
 *
 */

#ifndef CONSOLE_H__
#define CONSOLE_H__

#include <stdint.h>
#include <stdbool.h>

/* Newline character sequences */
#define CONSOLE_NEWLINE_CRLF          "\r\n" //!< CRLF newline 
#define CONSOLE_NEWLINE_CR            "\r" //!< CR newline
#define CONSOLE_NEWLINE_LF            "\n" //!< LF newline 
#define CONSOLE_NEWLINE_CRLF_NUMERIC  "\x0D\x0A"  //!< Hardcoded ASCII values for CRLF
#define CONSOLE_NEWLINE_CR_NUMERIC    "\x0D"      //!< It is possible, though unlikely, that
#define CONSOLE_NEWLINE_LF_NUMERIC    "\x0A"      //!< CR and LF do not equal these values.

#define CONSOLE_NEWLINE_DEFAULT       CONSOLE_NEWLINE_CRLF_NUMERIC   /*!< Default newline style */

/* Newline style for input */
#ifndef CONSOLE_NEWLINE_INPUT
  /* CONSOLE_NEWLINE_INPUT was not defined in console_config.h */
  #define CONSOLE_NEWLINE_INPUT      CONSOLE_NEWLINE_DEFAULT //!< Newline style for input 
#endif
  
/* Newline style for output */
#ifndef CONSOLE_NEWLINE_OUTPUT
  /* CONSOLE_NEWLINE_OUTPUT was not defined in console_config.h */
  #define CONSOLE_NEWLINE_OUTPUT     CONSOLE_NEWLINE_DEFAULT //!< Newline style for output 
#endif

/** 
 * @brief Function for initializing the console.
 * Init must be called prior to any other console functions.
 * The success of init() can be tested using the console_available() function.
 * console_init() is idempotent (can be called multiple times)
 * \post Interrupts required for the console I/O device (UART) are enabled.
*/
void console_init(void);

/** \brief Function for testing if the console is available.
 * 
 * \return true if console is initialised and I/O peripheral is initialised
 * \return false if console is not initialised or I/O peripheral init failed.
*/
bool console_available(void);


/** \brief Function for printing a string.
 *
 * Print a zero-terminated string of characters (octets).
 *
 * \param[in] string Pointer to the string.
 * \pre only works if init() previously called
 */
void console_put_string(uint8_t const  * string);


/** \brief Function for printing a string and newline.
 *
 * Print a zero-terminated string of characters (octets) 
 * and thereafter the output newline sequence.
 *
 * \param[in] string Pointer to the string.
 * \pre only works if init() previously called
 */
void console_put_line(uint8_t const  * string);


/** \brief Function for printing a newline (the newline output sequence).
 * \pre only works if init() previously called
 */
void console_put_newline(void);


/** \brief Function for printing a number of characters (octets).
 *
 * \param[in] chars Pointer to the characters
 * \param[in] num_chars Number of characters to print
 * \pre only works if init() previously called
 */ 
void console_put_chars(uint8_t const  * chars, uint8_t num_chars);


/** \brief Function for reading a number of characters (octets) and append zero-termination.
 *
 * \param[out] string Pointer to characters
 * \param[in] num_chars Number of characters to read.
 * \warning: Note that due to the appended zero termination, there must 
 * be room for num_chars + 1 octets in the space pointed to by argument chars.
 * \pre only works if init() previously called
 */ 
void console_get_string(uint8_t * string, uint8_t num_chars);

/** \brief Function for reading a line.
 *
 * Read characters until end of line or until a maximum number
 * of characters have been read, whatever comes first.
 * The characters read are returned as a zero-terminated string
 * (withouth the newline).
 *
 * \warning The function has a limitation in that it assumes that if the 
 * start of a newline sequence is found, the rest of the newline sequence must
 * follow.  The consequence of this is that if a leading part of the newline 
 * sequence (the "leading part") is found in the input, followed by something 
 * that is not the rest of the newline sequence, both the leading part and 
 * the first part of what follows will be silently discarded.
 * (The practical reason for this behaviour is the limitation on string length
 * given by the max_len parameter combined with the fact that hal_uart does not
 * provide a means of "putting back" already read characters.  It is 
 * possible to implement the function is such a way that this limitation would 
 * apply only when the string is too full to fit a number of characters equal to
 * the number of characters in a newline sequence.  Letting it apply all the time
 * seems more consistent, though.)
 *
 * \param[out] string Pointer to the string
 * \param[in] max_len Maximum length of string, including zero terminator.  
 * Note that max_len must be at least two (max_len >= 2).
 * \pre only works if init() previously called
 */
void console_get_line(uint8_t * string, uint8_t max_len);

/** \brief Function for getting number of characters sent to console.
 *
 *
 * \return Number of characters (0..255)
*/
bool console_chars_available(void);

/** \brief Function for reading a number of characters (octets).
 *
 * \param[out] chars Pointer to characters
 * \param[in] num_chars Number of characters to read
 * \pre only works if init() previously called
 */ 
void console_get_chars(uint8_t * chars, uint8_t num_chars);


/** \brief Function for writing a single character (octet).
 *
 * This function is nothing but a direct wrapper for hal_uart_putchar().
 * It is provided here for completeness only.  You may consider to use
 * hal_uart_putchar() directly instead.
 * \param ch Character to write
 * \pre only works if init() previously called
 */
void console_put_char(uint8_t ch);


/** \brief Function for reading a single character (octet).
 *
 * This function is a wrapper for hal_uart_getchar(), with the additional opption
 * of echo of input to output (if the console module is so configured, see the 
 * module documentation).
 * \pre only works if init() previously called
 * \return Read character
 */
uint8_t console_get_char(void);


/** \brief Function for printing the decimal ASCII representation of an 8-bit number.
 * 
 * \warning: This function is larger and more complex than the corresponding
 * functions to write hexadecimal representations.
 *
 * \param b Number in the range [0 255]
 * \pre only works if init() previously called
 */
void console_put_decbyte(uint8_t b);


/** \brief Function for printing the decimal ASCII representation of a 16-bit number.
 * 
 * \warning: This function is larger and more complex than the corresponding
 * functions to write hexadecimal representations.
 *
 * \param w Number in the range [0 65535]
 * \pre only works if init() previously called
 */
void console_put_decword(uint16_t w);


/** \brief Function for printing the decimal ASCII representation of a 32-bit number.
 * 
 * \warning: This function is larger and more complex than the corresponding
 * functions to write hexadecimal representations.
 *
 * \param ww Number in the range [0 4294967295]
 * \pre only works if init() previously called
 */
void console_put_dec32bit(uint32_t ww);


/** \brief Function for printing the hexadecimal ASCII representation of a nibble (half an octet, four bits).
 *
 * Four bits (a nibble) equals one hexadecimal digit.
 *
 * \param n A number from zero to 15, both inclusive
 * \pre only works if init() previously called
 */
void console_put_hexnybble(uint8_t n);


/** \brief Function for printing the hexadecimal ASCII representation of a byte (an octet).
 * 
 * \param b A byte (an octet)
 */
void console_put_hexbyte(uint8_t b);


/** \brief Function for printing the hexadecimal ASCII representation of a word (two octets).
 * 
 * \param w A word (sixten-bits, two octets)
 */
void console_put_hexword(uint16_t w);

/** \brief Function for printing a string of hexbytes.
 *
 * \param p a pointer to the first element
 * \param n the length of the array
 */
void console_put_hexbytearray(uint8_t* p, uint8_t n);

/** \brief Function for reading one hexadecimal digit.
 *
 * Read the hexadecimal ASCII representation of a nibble (half an octet, four bits)
 *
 * \warning: The function returns zero on invalid input (i.e. input that is not an
 * hexadecimal digit).
 *
 * \return A number from zero to 15, both inclusive
 * \pre only works if init() previously called
 */
uint8_t console_get_hexnybble(void);


/** \brief Function for reading two hexadecimal digits.
 *
 * Read the hexadecimal ASCII representation of a byte (an octet)
 *
 * \warning: Invalid input: Input characters that are not hexadecimal digits 
 * are treated as the hexadecimal digit zero.
 *
 * \return A number from zero to 255, both inclusive
 */
uint8_t console_get_hexbyte(void);


/** \brief Function for reading four hexadecimal digits.
 *
 * Read the hexadecimal ASCII representation of a word (16 bits)
 *
 * \warning: Invalid input: Input characters that are not hexadecimal digits 
 * are treated as the hexadecimal digit zero.
 *
 * \return A number from zero to 65535, both inclusive
 */
uint16_t console_get_hexword(void);

/** \brief Function for checking if the last tx is completed.
 *
 * \return TRUE, if the tx is completed, FALSE, is the Tx is in progress
 */
bool console_tx_completed(void);

#endif

