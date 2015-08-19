/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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

/**
 * @file
 * @brief File with example code presenting usage of drivers for TWIS slave and TWI in master mode
 *
 * @sa twi_master_with_twis_slave_example
 */

/**
 * @defgroup twi_master_with_twis_slave_example Example code presenting usage of drivers for TWIS slave and TWI in master mode
 *
 * This code presents usage of the two drivers:
 * - @ref nrf_twi_drv (in synchronous mode)
 * - @ref nrf_twis_drv (in asynchronous mode)
 *
 * On the slave an EEPROM memory is simulated.
 * For simulated EEPROM AT24C01C device produced by ATMEL was selected.
 * This device has 128 bytes of memory and it is simulated using internal RAM.
 * This RAM area is accessed only by simulated EEPROM so the rest of application can access it
 * only using TWI commands via hardware configured pins.
 *
 * Selected memory chip uses 7 bits constant address. Word to access is selected during
 * write operation: first byte send is used as current address pointer.
 *
 * Maximum 8 byte page can be written in single access.
 * The whole memory can be read in single access.
 *
 * Differences between real chip and simulated one:
 * 1. Simulated chip has practically 0ms write time.
 *    This example does not poll the memory for readiness.
 * 2. During sequential read, when memory end is reached, zeroes are send.
 *    There is no support for roll-over.
 * 3. It is possible to write maximum of 8 bytes in single sequential write.
 *    But in simulated EEPROM the whole address pointer is incremented.
 *    In real memory chip only 3 lowest bits changes during writing.
 *    In real device writing would roll-over in memory page.
 *
 * On the master side we communicate with that memory and allow write and read.
 * We can use simple commands via UART to check the memory.
 *
 * Pins to short:
 * - @ref TWI_SCL_M - @ref EEPROM_SIM_SCL_S
 * - @ref TWI_SDA_M - @ref EEPROM_SIM_SDA_S
 *
 * Supported commands are always listed in the welcome message.
 * @{
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include "config.h"
#include "eeprom_simulator.h"
#include "app_uart.h"
#include "nrf_drv_twi.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "nrf.h"
#include "bsp.h"
#include "app_util_platform.h"


/**
 * @brief Repeated part of help string
 *
 * This string contains list of supported command together with description.
 * It is used in welcome message and in help message if unsupported commmand is detected.
 */
static const char m_cmd_help_str[] =
        "   p - Print the EEPROM contents in a form: address, 8 bytes of code, ASCII form.\n"
        "   w - Write string starting from address 0.\n"
        "   c - Clear the memory (write 0xff)\n"
        "   x - Get transmission error byte.";


/**
 * @brief TWI master instance
 *
 * Instance of TWI master driver that would be used for communication with simulated
 * eeprom memory.
 */
static const nrf_drv_twi_t m_twi_master = NRF_DRV_TWI_INSTANCE(MASTER_TWI_INST);


/**
 * @brief Write data to simulated EEPROM
 *
 * Function uses TWI interface to write data into EEPROM memory.
 *
 * @param     addr  Start address to write
 * @param[in] pdata Pointer to data to send
 * @param     size  Byte count of data to send
 * @attention       Maximum number of bytes that may be written is @ref EEPROM_SIM_SEQ_WRITE_MAX.
 *                  In sequential write all data have to be in the same page
 *                  (higher address bits do not change).
 *
 * @return NRF_SUCCESS or reason of error.
 *
 * @attention If you wish to communicate with real EEPROM memory chip check its readiness
 * after writing data.
 */
static ret_code_t eeprom_write(size_t addr, uint8_t const * pdata, size_t size)
{
    ret_code_t ret;
    /* Memory device supports only limited number of bytes written in sequence */
    if((size > EEPROM_SIM_SEQ_WRITE_MAX) && (size > 0))
        return NRF_ERROR_INVALID_LENGTH;
    /* All written data has to be in the same page */
    if((addr/EEPROM_SIM_SEQ_WRITE_MAX) != ((addr+size-1)/EEPROM_SIM_SEQ_WRITE_MAX))
        return NRF_ERROR_INVALID_ADDR;
    do
    {
        uint8_t addr8 = (uint8_t)addr;
        ret = nrf_drv_twi_tx(&m_twi_master, EEPROM_SIM_ADDR, &addr8, 1, true);
        if(NRF_SUCCESS != ret)
        {
            break;
        }
        ret = nrf_drv_twi_tx(&m_twi_master, EEPROM_SIM_ADDR, pdata, size, false);
    }while(0);
    return ret;
}


/**
 * @brief Read data from simulated EEPROM
 *
 * Function uses TWI interface to read data from EEPROM memory.
 *
 * @param     addr  Start address to read
 * @param[in] pdata Pointer to the buffer to fill with data
 * @param     size  Byte count of data to read
 *
 * @return NRF_SUCCESS or reason of error.
 */
static ret_code_t eeprom_read(size_t addr, uint8_t * pdata, size_t size)
{
    ret_code_t ret;
    if(size > EEPROM_SIM_SIZE)
        return NRF_ERROR_INVALID_LENGTH;
    do
    {
       uint8_t addr8 = (uint8_t)addr;
       ret = nrf_drv_twi_tx(&m_twi_master, EEPROM_SIM_ADDR, &addr8, 1, true);
       if(NRF_SUCCESS != ret)
       {
           break;
       }
       ret = nrf_drv_twi_rx(&m_twi_master, EEPROM_SIM_ADDR, pdata, size, false);
    }while(0);
    return ret;
}


/**
 * @brief Put printable character to stdout
 *
 * Function puts given character to stdout.
 * If the character is not printable it is changed to dot before processing.
 *
 * @param c Character to print
 */
static void safe_putc(char c)
{
    if(!isprint((int)c))
    {
        c = '.';
    }
    UNUSED_VARIABLE(putc(c, stdout));
}


/**
 * @brief Print hexadecimal value to stdout
 *
 * Function prints given value as 2 digit hexadecimal.
 * Printed value always finishes with space.
 * This function is used in EEPROM content pretty-printing.
 *
 * @sa do_print_data
 *
 * @param data Value to print
 */
static void print_hex(uint8_t data)
{
    printf("%.2x ", (unsigned int)data);
}


/**
 * @brief Print address
 *
 * Function used to print address of a row in EEPROM content pretty-printing.
 * Function finishes printed value with a colon and a space.
 *
 * @sa do_print_data
 *
 * @param addr Address to print
 */
static void print_addr(size_t addr)
{
    printf("%.2x: ", (unsigned int)addr);
}


/**
 * @brief Pretty-print EEPROM content
 *
 * Respond on memory print command.
 */
static void do_print_data(void)
{
    size_t addr;
    uint8_t buff[IN_LINE_PRINT_CNT];
    for(addr=0; addr<EEPROM_SIM_SIZE; addr+=IN_LINE_PRINT_CNT)
    {
        unsigned int n;
        ret_code_t err_code;
        err_code = eeprom_read(addr, buff, IN_LINE_PRINT_CNT);
        APP_ERROR_CHECK(err_code);

        print_addr(addr);
        for(n=0; n<IN_LINE_PRINT_CNT; ++n)
        {
            print_hex(buff[n]);
        }

        safe_putc(' '); safe_putc(' ');

        for(n=0; n<IN_LINE_PRINT_CNT; ++n)
        {
            safe_putc((char)buff[n]);
        }
        UNUSED_VARIABLE(putc('\n', stdout));
    }
    UNUSED_VARIABLE(fflush(stdout));
}


/**
 * @brief Safely get string from stdin
 *
 * Function reads character by character into given buffer.
 * Maximum @em nmax number of characters are read.
 *
 * Function ignores all nonprintable characters.
 * String may be finished by CR or NL.
 *
 * @attention
 * Remember that after characters read zero would be added to mark the string end.
 * Given buffer should be no smaller than @em nmax + 1
 *
 * @param[out] str  Buffer for the string
 * @param      nmax Maximum number of characters to be readed
 */
static void safe_gets(char * str, size_t nmax)
{
    int c;
    while(1)
    {
        c = getchar();
        if(isprint(c))
        {
            *str++ = (char)c;
            UNUSED_VARIABLE(putc(c, stdout));
            UNUSED_VARIABLE(fflush(stdout));
            if(0 == --nmax)
                break;
        }
        else if('\n' == c || '\r' == c)
        {
            break;
        }
    }
    *str = '\0';
    UNUSED_VARIABLE(putc('\n', stdout));
    UNUSED_VARIABLE(fflush(stdout));
}

/**
 * @brief Check the string length check no more than given number of characters
 *
 * Function iterates through the string searching for the zero character.
 * Even when it is not found it stops iterating after maximum of @em nmax characters.
 *
 * @param[in] str  String to check
 * @param     nmax Maximum number of characters to check
 *
 * @return String length or @em nmax if no the zero character is found.
 */
static size_t safe_strlen(char const * str, size_t nmax)
{
    size_t n=0;
    while('\0' != *str++)
    {
        ++n;
        if(0 == --nmax)
            break;
    }
    return n;
}

/**
 * @brief Function that performs the command of writing string to EEPROM
 *
 * Function gets user input and writes given string to EEPROM starting from address 0.
 * It is accessing EEPROM using maximum allowed number of bytes in sequence.
 */
static void do_string_write(void)
{
    char str[EEPROM_SIM_SIZE+1];
    size_t addr = 0;

    UNUSED_VARIABLE(puts("Waiting for string to write:"));
    safe_gets(str, sizeof(str)-1);
    while(1)
    {
        ret_code_t err_code;
        size_t to_write = safe_strlen(str+addr, EEPROM_SIM_SEQ_WRITE_MAX);
        if(0 == to_write)
            break;
        err_code = eeprom_write(addr, (uint8_t const *)str+addr, to_write);
        APP_ERROR_CHECK(err_code);
        addr += to_write;
    }
}

/**
 * @brief Function that performs simulated EEPROM clearing
 *
 * Function fills the EEPROM with 0xFF value.
 * It is accessing the EEPROM writing only one byte at once.
 */
static void do_clear_eeprom(void)
{
    uint8_t clear_val = 0xff;
    size_t addr;
    for(addr=0; addr<EEPROM_SIM_SIZE; ++addr)
    {
        ret_code_t err_code;
        err_code = eeprom_write(addr, &clear_val, 1);
        APP_ERROR_CHECK(err_code);
    }
    UNUSED_VARIABLE(puts("Memory erased"));
    UNUSED_VARIABLE(fflush(stdout));
}

/**
 * @brief Initialize the master TWI
 *
 * Function used to initialize master TWI interface that would communicate with simulated EEPROM.
 *
 * @return NRF_SUCCESS or the reason of failure
 */
static ret_code_t twi_master_init(void)
{
    ret_code_t ret;
    const nrf_drv_twi_config_t config =
    {
       .scl                = TWI_SCL_M,
       .sda                = TWI_SDA_M,
       .frequency          = NRF_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH
    };

    do
    {
        ret = nrf_drv_twi_init(&m_twi_master, &config, NULL);
        if(NRF_SUCCESS != ret)
        {
            break;
        }
        nrf_drv_twi_enable(&m_twi_master);
    }while(0);
    return ret;
}

/**
 * @brief Handle UART errors
 *
 * Simple function for handling any error from UART module.
 * See UART example for more information.
 *
 * @param[in] p_event Event structure
 */
static void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}




/**
 *  The begin of the journey
 */
int main(void)
{
    ret_code_t err_code;
    /* Initialization of UART */
    LEDS_CONFIGURE(LEDS_MASK);
    LEDS_OFF(LEDS_MASK);

    const app_uart_comm_params_t comm_params =
    {
        RX_PIN_NUMBER,
        TX_PIN_NUMBER,
        RTS_PIN_NUMBER,
        CTS_PIN_NUMBER,
        APP_UART_FLOW_CONTROL_ENABLED,
        false,
        UART_BAUDRATE_BAUDRATE_Baud38400
    };

    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_error_handle,
                       APP_IRQ_PRIORITY_LOW,
                       err_code);

    APP_ERROR_CHECK(err_code);

    /* Initializing simulated EEPROM */
    err_code = eeprom_simulator_init();
    APP_ERROR_CHECK(err_code);

    /* Initializing TWI master interface for EEPROM */
    err_code = twi_master_init();
    APP_ERROR_CHECK(err_code);


    /* Welcome message */
    UNUSED_VARIABLE(puts(
            "This is TWIS and TWI usage example\n"
            "You can access simulated EEPROM memory using following commands:"
    ));
    UNUSED_VARIABLE(puts(m_cmd_help_str));
    
    UNUSED_VARIABLE(fflush(stdout));

    /* Main loop */
    while(1)
    {
        uint8_t c;
        while(NRF_SUCCESS != app_uart_get(&c))
        {
            // Just waiting
        }
        switch((char)c)
        {
        case '\n':
        case '\r':
            break;
        case 'p':
            do_print_data();
            break;
        case 'w':
            do_string_write();
            break;
        case 'c':
            do_clear_eeprom();
            break;
        case 'x':
            {
                uint32_t error = eeprom_simulator_error_get_and_clear();
                printf("Error word: %x\n", (unsigned int)error);
            }
            break;
        default:
            printf("You selected %c\n", (char)c);
            UNUSED_VARIABLE(puts("Unknown command, try one of the following:"));
            UNUSED_VARIABLE(puts(m_cmd_help_str));
            break;
        }
        if(eeprom_simulator_error_check())
        {
            UNUSED_VARIABLE(puts(
                    "WARNING: EEPROM transmission error detected.\n"
                    "Use 'x' command to read error word."
            ));
            UNUSED_VARIABLE(fflush(stdout));
        }
    }
}

/** @} */ /* End of group twi_master_with_twis_slave_example */
