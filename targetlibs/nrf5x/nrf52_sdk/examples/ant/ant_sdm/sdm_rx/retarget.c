/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
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

#include "app_uart.h" 
#include <stdio.h>
#include <stdint.h>
#include "nordic_common.h"

#if !defined(__ICCARM__)
struct __FILE { int handle; };
FILE __stdout;
FILE __stdin;
#endif

int fputc(int ch, FILE * p_file) 
{
#if defined(TRACE_UART)
    const uint32_t err_code = app_uart_put((uint8_t)ch);
    UNUSED_VARIABLE(err_code);
#endif // TRACE_UART

    return ch;
}


int _write(int file, const char * p_data, int len)
{
#if defined(TRACE_UART)
    uint32_t err_code;
    
    const char * p_end_of_sequence = (p_data + len);
    
    while (p_data != p_end_of_sequence)
    {
        err_code = app_uart_put(*p_data++);
        UNUSED_VARIABLE(err_code);          
    }
#endif // TRACE_UART

    return len;
}
