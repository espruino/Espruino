/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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
#include "sdk_config.h"
#if NRF_LOG_ENABLED
#include "nrf_log_backend.h"
#include "nrf_error.h"
#include "nordic_common.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#if NRF_LOG_BACKEND_SERIAL_USES_RTT
#include <SEGGER_RTT_Conf.h>
#include <SEGGER_RTT.h>
#endif

#if NRF_LOG_BACKEND_SERIAL_USES_UART
#include "nrf_drv_uart.h"
#endif

#if NRF_LOG_BACKEND_SERIAL_USES_RTT
static char m_rtt_buffer[NRF_LOG_BACKEND_MAX_STRING_LENGTH];
#endif

#if NRF_LOG_BACKEND_SERIAL_USES_UART
static char m_uart_buffer[NRF_LOG_BACKEND_MAX_STRING_LENGTH];
static nrf_drv_uart_t m_uart = NRF_DRV_UART_INSTANCE(NRF_LOG_BACKEND_UART_INSTANCE);

#if (UART_ENABLED == 0)
#error "UART driver must be enabled to use UART in nrf_log."
#endif

#endif //NRF_LOG_BACKEND_SERIAL_USES_UART

#define HEXDUMP_BYTES_PER_LINE               16
#define HEXDUMP_HEXBYTE_AREA                 3 // Two bytes for hexbyte and space to separate
#define TIMESTAMP_STR(val) "[%0" NUM_TO_STR(val) "d]"

#define RTT_RETRY_COUNTER 10 //Number of retries before skipping processing

#define HEXDUMP_MAX_STR_LEN (NRF_LOG_BACKEND_MAX_STRING_LENGTH -          \
                            (HEXDUMP_HEXBYTE_AREA*HEXDUMP_BYTES_PER_LINE +\
                             NRF_LOG_TIMESTAMP_DIGITS +                   \
                             4 +/* Color ANSI Escape Code */              \
                             2)) /* Separators */

static bool m_initialized   = false;
static bool m_blocking_mode = false;

#if (NRF_LOG_BACKEND_SERIAL_USES_UART)
static volatile bool m_rx_done = false;
#endif

#if (NRF_LOG_BACKEND_SERIAL_USES_UART)
static void uart_event_handler(nrf_drv_uart_event_t * p_event, void * p_context)
{
    // Dummy handler since is_busy feature is used for determining readiness.
    if (p_event->type == NRF_DRV_UART_EVT_RX_DONE)
    {
        m_rx_done = true;
    }
}
#endif //NRF_LOG_BACKEND_SERIAL_USES_UART


ret_code_t nrf_log_backend_init(bool blocking)
{
    uint32_t              ret_code;

    if (m_initialized && (blocking == m_blocking_mode))
    {
        return NRF_SUCCESS;
    }

#if NRF_LOG_BACKEND_SERIAL_USES_RTT
    ret_code = SEGGER_RTT_ConfigUpBuffer(
        0,
        "Normal",
        m_rtt_buffer,
        NRF_LOG_BACKEND_MAX_STRING_LENGTH,
        SEGGER_RTT_MODE_NO_BLOCK_TRIM);
    if ( ret_code != 0)
    {
        return NRF_ERROR_INVALID_STATE;
    }
#endif //NRF_LOG_BACKEND_SERIAL_USES_RTT

#if (NRF_LOG_BACKEND_SERIAL_USES_UART)
    nrf_drv_uart_config_t uart_config = NRF_DRV_UART_DEFAULT_CONFIG;
    uart_config.hwfc     =
            (nrf_uart_hwfc_t)NRF_LOG_BACKEND_SERIAL_UART_FLOW_CONTROL;
    uart_config.pseltxd  = NRF_LOG_BACKEND_SERIAL_UART_TX_PIN;
    uart_config.pselrxd  = NRF_LOG_BACKEND_SERIAL_UART_RX_PIN;
    uart_config.pselrts  = NRF_LOG_BACKEND_SERIAL_UART_RTS_PIN;
    uart_config.pselcts  = NRF_LOG_BACKEND_SERIAL_UART_CTS_PIN;
    uart_config.baudrate =
        (nrf_uart_baudrate_t)NRF_LOG_BACKEND_SERIAL_UART_BAUDRATE;
    nrf_drv_uart_uninit(&m_uart);
    ret_code = nrf_drv_uart_init(&m_uart, &uart_config,
                                 blocking ? NULL : uart_event_handler);
    if (ret_code != NRF_SUCCESS)
    {
        return ret_code;
    }
#endif //NRF_LOG_BACKEND_SERIAL_USES_UART

    m_initialized   = true;
    m_blocking_mode = blocking;
    return NRF_SUCCESS;
}


static bool serial_is_busy(void)
{
    bool res = false;

#if (NRF_LOG_BACKEND_SERIAL_USES_UART)
    res = nrf_drv_uart_tx_in_progress(&m_uart);
#endif

#if (NRF_LOG_BACKEND_SERIAL_USES_RTT)

#endif

    return res;
}


static bool serial_tx(uint8_t * p_buf, uint32_t len)
{
    bool ret = true;

#if NRF_LOG_BACKEND_SERIAL_USES_UART
    memcpy(m_uart_buffer, p_buf, len);
    uint32_t ret_code = nrf_drv_uart_tx(&m_uart, (uint8_t *)m_uart_buffer, len);
    if (ret_code != NRF_SUCCESS)
    {
        ret = false;
    }
#endif //NRF_LOG_BACKEND_SERIAL_USES_UART

#if NRF_LOG_BACKEND_SERIAL_USES_RTT
    uint32_t idx    = 0;
    uint32_t length = len;
    uint32_t processed;
    uint32_t watchdog_counter = RTT_RETRY_COUNTER;
    do
    {
        processed = SEGGER_RTT_WriteNoLock(0, &p_buf[idx], length);
        idx += processed;
        length -= processed;
        if (processed == 0)
        {
            // If RTT is not connected then ensure that logger does not block
            watchdog_counter--;
            if (watchdog_counter == 0)
            {
                break;
            }
        }
    } while (length);
#endif //NRF_LOG_BACKEND_SERIAL_USES_RTT
    return ret;
}


static uint8_t serial_get_byte(void)
{
    uint8_t data;
#if NRF_LOG_BACKEND_SERIAL_USES_UART
    if (m_blocking_mode)
    {
        (void)nrf_drv_uart_rx(&m_uart, &data, 1);
    }
    else
    {
        m_rx_done = false;
        (void)nrf_drv_uart_rx(&m_uart, &data, 1);
        while(!m_rx_done);
    }
#elif NRF_LOG_BACKEND_SERIAL_USES_RTT
    data = (uint8_t)SEGGER_RTT_WaitKey();
#endif //NRF_LOG_BACKEND_SERIAL_USES_RTT
    return data;
}


static bool buf_len_update(uint32_t * p_buf_len, int32_t new_len)
{
    bool ret;
    if (new_len < 0)
    {
        ret = false;
    }
    else
    {
        *p_buf_len += (uint32_t)new_len;
        ret = true;
    }
    return ret;
}


static bool timestamp_process(const uint32_t * const p_timestamp, char * p_str, uint32_t * p_len)
{
    int32_t len = 0;
    bool    ret = true;
    if (p_timestamp)
    {
#if NRF_LOG_USES_COLORS
        char color[] = "\x1B[0m";
        len = sizeof(color) - 1;
        memcpy(p_str, color, len);
        *p_len += len;
#endif //NRF_LOG_USES_COLORS
        len = sprintf(&p_str[len], TIMESTAMP_STR(NRF_LOG_TIMESTAMP_DIGITS), (int)*p_timestamp);
        ret = buf_len_update(p_len, len);
    }
    else
    {
        *p_len = 0;
    }
    return ret;
}


static bool nrf_log_backend_serial_std_handler(
    uint8_t                severity_level,
    const uint32_t * const p_timestamp,
    const char * const     p_str,
    uint32_t             * p_args,
    uint32_t               nargs)
{
    char     str[NRF_LOG_BACKEND_MAX_STRING_LENGTH];
    int32_t  tmp_str_len     = 0;
    uint32_t buffer_len      = 0;
    bool     status          = true;

    if (serial_is_busy())
    {
        return false;
    }

    if (!timestamp_process(p_timestamp, &str[buffer_len], &buffer_len))
    {
        return false;
    }

    switch (nargs)
    {
        case 0:
        {
            tmp_str_len = strlen(p_str);
            if ((tmp_str_len + buffer_len) < NRF_LOG_BACKEND_MAX_STRING_LENGTH)
            {
                memcpy(&str[buffer_len], p_str, tmp_str_len);
            }
            break;
        }

        case 1:
            tmp_str_len = sprintf(&str[buffer_len], p_str, p_args[0]);

            break;

        case 2:
            tmp_str_len = sprintf(&str[buffer_len], p_str, p_args[0], p_args[1]);
            break;

        case 3:
            tmp_str_len = sprintf(&str[buffer_len], p_str, p_args[0], p_args[1], p_args[2]);
            break;

        case 4:
            tmp_str_len =
                sprintf(&str[buffer_len], p_str, p_args[0], p_args[1], p_args[2], p_args[3]);
            break;

        case 5:
            tmp_str_len =
                sprintf(&str[buffer_len],
                        p_str,
                        p_args[0],
                        p_args[1],
                        p_args[2],
                        p_args[3],
                        p_args[4]);
            break;

        case 6:
            tmp_str_len =
                sprintf(&str[buffer_len],
                        p_str,
                        p_args[0],
                        p_args[1],
                        p_args[2],
                        p_args[3],
                        p_args[4],
                        p_args[5]);
            break;

        default:
            break;
    }
    status = buf_len_update(&buffer_len, tmp_str_len);
    if (!status || buffer_len > NRF_LOG_BACKEND_MAX_STRING_LENGTH)
    {
        // error, sprintf failed.
        return false;
    }
    else
    {
        return serial_tx((uint8_t *)str, buffer_len);
    }
}


static void byte2hex(const uint8_t c, char * p_out)
{
    uint8_t  nibble;
    uint32_t i = 2;

    while (i-- != 0)
    {
        nibble       = (c >> (4 * i)) & 0x0F;
        p_out[1 - i] = (nibble > 9) ? ('A' + nibble - 10) : ('0' + nibble);
    }
}


static char char2readablechar(char c)
{
    if ((c >= 32) && (c <= 126))
    {
        return c;
    }
    else
    {
        return '.';
    }
}


static uint32_t nrf_log_backend_serial_hexdump_handler(
    uint8_t                severity_level,
    const uint32_t * const p_timestamp,
    const char * const     p_str,
    uint32_t               offset,
    const uint8_t * const  p_buf0,
    uint32_t               buf0_length,
    const uint8_t * const  p_buf1,
    uint32_t               buf1_length)
{
    char     str[NRF_LOG_BACKEND_MAX_STRING_LENGTH];
    uint32_t slen;
    char   * p_hex_part;
    char   * p_char_part;
    uint8_t  c;
    uint32_t byte_in_line;
    uint32_t buffer_len    = 0;
    uint32_t byte_cnt      = offset;
    uint32_t length        = buf0_length + buf1_length;
    uint32_t timestamp_len = p_timestamp ?
            NRF_LOG_TIMESTAMP_DIGITS+2 : 0; //+2 since timestamp is in brackets

    if (serial_is_busy())
    {
        return offset;
    }

    // If it is the first part of hexdump print the header
    if (offset == 0)
    {
        if (!timestamp_process(p_timestamp, &str[buffer_len], &buffer_len))
        {
            return offset;
        }
        slen = strlen(p_str);
        // Saturate string if it's too long.
        slen = (slen > HEXDUMP_MAX_STR_LEN) ? HEXDUMP_MAX_STR_LEN : slen;
        memcpy(&str[buffer_len], p_str, slen);
        buffer_len += slen;
    }

    do
    {

        uint32_t  i;
        uint32_t hex_part_offset  = buffer_len;
        uint32_t char_part_offset = hex_part_offset +
                                    (HEXDUMP_BYTES_PER_LINE * HEXDUMP_HEXBYTE_AREA + 1) + // +1 - separator between hexdump and characters.
                                    timestamp_len;

        p_hex_part  = &str[hex_part_offset];
        p_char_part = &str[char_part_offset];

        // Fill the blanks to align to timestamp print
        for (i = 0; i < timestamp_len; i++)
        {
            *p_hex_part = ' ';
            ++p_hex_part;
        }

        for (byte_in_line = 0; byte_in_line < HEXDUMP_BYTES_PER_LINE; byte_in_line++)
        {
            if (byte_cnt >= length)
            {
                // file the blanks
                *p_hex_part++  = ' ';
                *p_hex_part++  = ' ';
                *p_hex_part++  = ' ';
                *p_char_part++ = ' ';
            }
            else
            {
                if (byte_cnt < buf0_length)
                {
                    c = p_buf0[byte_cnt];
                }
                else
                {
                    c = p_buf1[byte_cnt - buf0_length];
                }
                byte2hex(c, p_hex_part);
                p_hex_part    += 2; // move the pointer since byte in hex was added.
                *p_hex_part++  = ' ';
                *p_char_part++ = char2readablechar(c);
                byte_cnt++;
            }
        }
        *p_char_part++ = '\r';
        *p_char_part++ = '\n';
        *p_hex_part++  = ' ';
        buffer_len    += timestamp_len +
                         (HEXDUMP_BYTES_PER_LINE * HEXDUMP_HEXBYTE_AREA + 1) + // space for hex dump and separator between hexdump and string
                         HEXDUMP_BYTES_PER_LINE +                              // space for stringS dump
                         2;                                                    // space for new line
        if (!serial_tx((uint8_t *)str, buffer_len))
        {
            return byte_cnt;
        }

        if (serial_is_busy())
        {
            return byte_cnt;
        }
        buffer_len = 0;
    }
    while (byte_cnt < length);
    return byte_cnt;
}


nrf_log_std_handler_t nrf_log_backend_std_handler_get(void)
{
    return nrf_log_backend_serial_std_handler;
}


nrf_log_hexdump_handler_t nrf_log_backend_hexdump_handler_get(void)
{
    return nrf_log_backend_serial_hexdump_handler;
}


uint8_t nrf_log_backend_getchar(void)
{
    return serial_get_byte();
}

#endif // NRF_LOG_ENABLED
