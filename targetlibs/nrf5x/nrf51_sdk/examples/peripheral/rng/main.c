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
 * @defgroup rng_example_main main.c
 * @{
 * @ingroup rng_example
 * @brief Random Number Generator Example Application main file.
 *
 */


#include <stdio.h>
#include <stdint.h>
#include "bsp.h"
#include "nrf_delay.h"
#include "app_uart.h"
#include "app_error.h"
#include "nrf_drv_rng.h"
#include "nrf_assert.h"

#ifdef SOFTDEVICE_PRESENT
#include "softdevice_handler.h"
#endif // SOFTDEVICE_PRESENT

#define UART_TX_BUF_SIZE 256                                                          /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 1                                                            /**< UART RX buffer size. */
#define RANDOM_BUFF_SIZE 16                                                           /**< Random numbers buffer size. */

extern void softdevice_assertion_handler(uint32_t pc, uint16_t line_num, const uint8_t * file_name);

void assert_nrf_callback(uint16_t line_num, const uint8_t *file_name)
{
    /* empty function - needed by softdevice handler */
}

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

/** @brief Function for getting vector of random numbers.
 *
 * @param[out] p_buff                               Pointer to unit8_t buffer for storing the bytes.
 * @param[in]  length                               Number of bytes to take from pool and place in p_buff.
 *
 * @retval     Number of bytes actually placed in p_buff.
 */
uint8_t random_vector_generate(uint8_t * p_buff, uint8_t size)
{
    uint8_t available;
    uint32_t err_code;
    err_code = nrf_drv_rng_bytes_available(&available);
    APP_ERROR_CHECK(err_code);
    uint8_t length = (size<available) ? size : available;
    err_code = nrf_drv_rng_rand(p_buff,length);
    APP_ERROR_CHECK(err_code);
    return length;
}

/** @brief Function for main application entry.
 */
int main(void)
{
    uint32_t err_code;
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

#ifdef SOFTDEVICE_PRESENT
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, false);
#endif // SOFTDEVICE_PRESENT

    err_code = nrf_drv_rng_init(NULL);
    APP_ERROR_CHECK(err_code);
    
    while (true)
    {
        uint8_t p_buff[RANDOM_BUFF_SIZE];
        uint8_t length = random_vector_generate(p_buff,RANDOM_BUFF_SIZE);
        printf("Random Vector:");
        for(uint8_t i = 0; i < length; i++)
        {
            printf(" %3d",(int)p_buff[i]);
        }
        printf("\n\r");
        nrf_delay_ms(100);
    }
}


/** @} */
