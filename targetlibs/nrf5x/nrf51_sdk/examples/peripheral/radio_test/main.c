/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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

/** @file
*
* @defgroup nrf_radio_test_example_main main.c
* @{
* @ingroup nrf_radio_test_example
* @brief Radio Test Example Application main file.
*
* This file contains the source code for a sample application using the NRF_RADIO, and is controlled through the serial port.
*
*/


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "bsp.h"
#include "nrf.h"
#include "radio_test.h"
#include "app_uart.h"
#include "app_error.h"
#include "nrf51_bitfields.h"
#include "nordic_common.h"

static uint8_t mode_          = RADIO_MODE_MODE_Nrf_2Mbit;
static uint8_t txpower_       = RADIO_TXPOWER_TXPOWER_0dBm;
static int channel_start_     = 0;
static int channel_end_       = 80;
static int delayms_           = 10;

static bool sweep = false;

typedef enum
{
    RADIO_TEST_NOP,      /**< No test running.      */
    RADIO_TEST_TXCC,     /**< TX constant carrier.  */
    RADIO_TEST_TXMC,     /**< TX modulated carrier. */
    RADIO_TEST_TXSWEEP,  /**< TX sweep.             */
    RADIO_TEST_RXC,      /**< RX constant carrier.  */
    RADIO_TEST_RXSWEEP,  /**< RX sweep.             */
} radio_tests_t;


#define BELL 7 // Bell

#define UART_TX_BUF_SIZE 512                                                          /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 1                                                            /**< UART RX buffer size. */

void uart_error_handle(app_uart_evt_t * p_event)
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

/** @brief Function for configuring all peripherals used in this example.
*/
static void init(void)
{
    NRF_RNG->TASKS_START = 1;
    
    // Start 16 MHz crystal oscillator
    NRF_CLOCK->EVENTS_HFCLKSTARTED  = 0;
    NRF_CLOCK->TASKS_HFCLKSTART     = 1;

    // Wait for the external oscillator to start up
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0)
    {
        // Do nothing.
    }  
}


/** @brief Function for outputting usage info to the serial port.
*/
static void help(void)
{
    printf("Usage:\r\n");
    printf("a: Enter start channel for sweep/channel for constant carrier\r\n");
    printf("b: Enter end channel for sweep\r\n");
    printf("c: Start TX carrier\r\n");
    printf("d: Enter time on each channel (1ms-99ms)\r\n");
    printf("e: Cancel sweep/carrier\r\n");
    printf("m: Enter data rate\r\n");
    printf("o: Start modulated TX carrier\r\n");
    printf("p: Enter output power\r\n");
    printf("s: Print current delay, channels and so on\r\n");
    printf("r: Start RX sweep\r\n");
    printf("t: Start TX sweep\r\n");
    printf("x: Start RX carrier\r\n");
}


/** @brief Function for reading the data rate.
*/
void get_datarate(void)
{
    uint8_t c;

    printf("Enter data rate ('0'=250 Kbit/s, '1'=1 Mbit/s and '2'=2 Mbit/s):\r\n");
    while (true)
    {
        scanf("%c",&c);
        if ((c >= '0') && (c <= '2'))
        {
            printf("%c\r\n",c);
            break;
        }
        else
        {
            printf("%c\r\n",BELL);
        }
    }
    if (c == '0')
    {
        mode_ = RADIO_MODE_MODE_Nrf_250Kbit;
    }
    else if (c == '1')
    {
        mode_ = RADIO_MODE_MODE_Nrf_1Mbit;
    }
    else
    {
        mode_ = RADIO_MODE_MODE_Nrf_2Mbit;
    }
    printf("\r\n");
}


/** @brief Function for reading the output power.
*/
void get_power(void)
{
    uint8_t c;

    printf("Enter output power ('0'=+4 dBm, '1'=0 dBm,...,'7'=-30 dBm):\r\n");
    while (true)
    {
        scanf("%c",&c);
        if ((c >= '0') && (c <= '7'))
        {
            UNUSED_VARIABLE(app_uart_put(c));
            break;
        }
        else
        {
            UNUSED_VARIABLE(app_uart_put(BELL));
        }
    }
    
    switch(c)
    {
        case '0':
            txpower_ =  RADIO_TXPOWER_TXPOWER_Pos4dBm;
            break;
        
        case '1':
            txpower_ =  RADIO_TXPOWER_TXPOWER_0dBm;
            break;
        
        case '2':
            txpower_ = RADIO_TXPOWER_TXPOWER_Neg4dBm;
            break;
        
        case '3':
            txpower_ = RADIO_TXPOWER_TXPOWER_Neg8dBm;
            break;
        
        case '4':
            txpower_ = RADIO_TXPOWER_TXPOWER_Neg12dBm;
            break;
        
        case '5':
            txpower_ = RADIO_TXPOWER_TXPOWER_Neg16dBm;
            break;
        
        case '6':
            txpower_ = RADIO_TXPOWER_TXPOWER_Neg20dBm;
            break;
        
        case '7':
            // fall through 
        
        default:
            txpower_ = RADIO_TXPOWER_TXPOWER_Neg30dBm;
            break;
    }
    printf("\r\n");
}


/** @brief Function for printing parameters to the serial port.
*/
void print_parameters(void)
{
    printf("Parameters:\r\n");
    switch(mode_)
    {
        case RADIO_MODE_MODE_Nrf_250Kbit:
            printf("Data rate...........: 250 Kbit/s\r\n");
            break;
        
        case RADIO_MODE_MODE_Nrf_1Mbit:
            printf("Data rate...........: 1 Mbit/s\r\n");
            break;
        
        case RADIO_MODE_MODE_Nrf_2Mbit:
            printf("Data rate...........: 2 Mbit/s\r\n");
            break;
    }
    
    switch(txpower_)
    {
        case RADIO_TXPOWER_TXPOWER_Pos4dBm:
            printf("TX Power............: +4 dBm\r\n");
            break;
        
        case RADIO_TXPOWER_TXPOWER_0dBm:
            printf("TX Power............: 0 dBm\r\n");
            break;
        
        case RADIO_TXPOWER_TXPOWER_Neg4dBm:
            printf("TX Power............: -4 dBm\r\n");
            break;
        
        case RADIO_TXPOWER_TXPOWER_Neg8dBm:
            printf("TX Power............: -8 dBm\r\n");
            break;
        
        case RADIO_TXPOWER_TXPOWER_Neg12dBm:
            printf("TX Power............: -12 dBm\r\n");
            break;
        
        case RADIO_TXPOWER_TXPOWER_Neg16dBm:
            printf("TX Power............: -16 dBm\r\n");
            break;
        
        case RADIO_TXPOWER_TXPOWER_Neg20dBm:
            printf("TX Power............: -20 dBm\r\n");
            break;
        
        case RADIO_TXPOWER_TXPOWER_Neg30dBm:
            printf("TX Power............: -30 dBm\r\n");
            break;
        
        default:
            // No implementation needed.
            break;
        
    }
    printf("(Start) Channel.....: %d\r\n",channel_start_);
    printf("End Channel.........: %d\r\n",channel_end_);
    printf("Time on each channel: %d\r\n",delayms_);
    printf(" ms\r\n");
}


/** @brief Function for main application entry.
 */
int main(void)
{ 
    uint32_t err_code;
    radio_tests_t test     = RADIO_TEST_NOP;
    radio_tests_t cur_test = RADIO_TEST_NOP;

    init();
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
    printf("RF Test\r\n");
    NVIC_EnableIRQ(TIMER0_IRQn);
    __enable_irq();
    while (true)
    {
        uint8_t control;
        scanf("%c",&control);
        switch (control)
        {
            case 'a':
                while (true)
                {
                    printf("Enter start channel (two decimal digits, 00 to 80):\r\n");
                    scanf("%d",&channel_start_);
                    if ((channel_start_ <= 80)&&(channel_start_ >= 0))
                    {
                        printf("%d\r\n", channel_start_);
                        break;
                    }

                    printf("Channel must be between 0 and 80\r\n");
                }
                test = cur_test;
                break;

            case 'b':
                while (true)
                {
                    printf("Enter end channel \
                            (two decimal digits, 00 to 80):\r\n");
                    scanf("%d",&channel_end_);
                    if ((channel_end_ <= 80)&&(channel_start_ >= 0))
                    {
                        printf("%d\r\n", channel_end_);
                        break;
                    }
                    printf("Channel must be between 0 and 80\r\n");
                }
                test = cur_test;
                break;

            case 'c':
                test = RADIO_TEST_TXCC;
                break;

            case 'd':
                while (true)
                {
                    printf("Enter delay in ms (two decimal digits, 01 to 99):\r\n");
                    scanf("%d",&delayms_);
                    if ((delayms_ > 0) && (delayms_ < 100))   
                    {
                        printf("%d\r\n", delayms_);
                        break;
                    }
                    printf("Delay must be between 1 and 99\r\n");
                }
                test = cur_test;
                break;

            case 'e':
                radio_sweep_end();
                cur_test = RADIO_TEST_NOP;
                break;

            case 'm':
                get_datarate();
                test = cur_test;
                break;

            case 'o':
                test = RADIO_TEST_TXMC;
                printf("TX modulated carrier\r\n");
                break;

            case 'p':
                get_power();
                test = cur_test;
                break;

            case 'r':
                test = RADIO_TEST_RXSWEEP;
                printf("RX Sweep\r\n");
                break;

            case 's':
                print_parameters();
                break;

            case 't':
                test = RADIO_TEST_TXSWEEP;
                printf("TX Sweep\r\n");
                break;

            case 'x':
                test = RADIO_TEST_RXC;
                printf("RX constant carrier\r\n");
                break;

            case 'h':
                // Fall through.
        
            default:
                help();
                break;
        }
    
        switch (test)
        {
            case RADIO_TEST_TXCC:
                if (sweep)
                {
                    radio_sweep_end();
                    sweep = false;
                }
                radio_tx_carrier(txpower_, mode_, channel_start_);
                cur_test = test;
                test     = RADIO_TEST_NOP;
                break;

            case RADIO_TEST_TXMC:
                if (sweep)
                {
                    radio_sweep_end();
                    sweep = false;
                }
                radio_modulated_tx_carrier(txpower_, mode_, channel_start_);
                cur_test = test;
                test     = RADIO_TEST_NOP;
                break;

            case RADIO_TEST_TXSWEEP:
                radio_tx_sweep_start(txpower_, mode_, channel_start_, channel_end_, delayms_);
                sweep    = true;
                cur_test = test;
                test     = RADIO_TEST_NOP;
                break;

            case RADIO_TEST_RXC:
                if (sweep)
                {
                    radio_sweep_end();
                    sweep = false;
                }
                radio_rx_carrier(mode_, channel_start_);
                cur_test = test;
                test     = RADIO_TEST_NOP;
                break;  

            case RADIO_TEST_RXSWEEP:
                radio_rx_sweep_start(mode_, channel_start_, channel_end_, delayms_);
                sweep    = true;
                cur_test = test;
                test     = RADIO_TEST_NOP;
                break;

            case RADIO_TEST_NOP:
                // Fall through.
            default:
                // No implementation needed.
                break;
        }
    }
}

/** @} */
