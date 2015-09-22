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
* @defgroup ppi_example_main main.c
* @{
* @ingroup ppi_example
* @brief PPI Example Application main file.
*
* This file contains the source code for a sample application using PPI to communicate between timers.
*
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "nrf_delay.h"
#include "app_uart.h"
#include "app_error.h"
#include "boards.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "nordic_common.h"

#define UART_TX_BUF_SIZE 256                                                          /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 1                                                            /**< UART RX buffer size. */

const nrf_drv_timer_t timer0 = NRF_DRV_TIMER_INSTANCE(0);
const nrf_drv_timer_t timer1 = NRF_DRV_TIMER_INSTANCE(1);
const nrf_drv_timer_t timer2 = NRF_DRV_TIMER_INSTANCE(2);

nrf_ppi_channel_t ppi_channel1, ppi_channel2;

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

// Timer even handler. Not used since timer is used only for PPI.
void timer_event_handler(nrf_timer_event_t event_type, void * p_context){}

/** @brief Function for initializing the PPI peripheral.
*/
static void ppi_init(void)
{
    uint32_t err_code = NRF_SUCCESS;

    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);

    // Configure 1st available PPI channel to stop TIMER0 counter on TIMER1 COMPARE[0] match, which is every even number of seconds.
    err_code = nrf_drv_ppi_channel_alloc(&ppi_channel1);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_ppi_channel_assign(ppi_channel1,
                                          nrf_drv_timer_event_address_get(&timer1, NRF_TIMER_EVENT_COMPARE0),
                                          nrf_drv_timer_task_address_get(&timer0, NRF_TIMER_TASK_STOP));
    APP_ERROR_CHECK(err_code);

    // Configure 2nd available PPI channel to start timer0 counter at TIMER2 COMPARE[0] match, which is every odd number of seconds.
    err_code = nrf_drv_ppi_channel_alloc(&ppi_channel2);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_ppi_channel_assign(ppi_channel2,
                                          nrf_drv_timer_event_address_get(&timer2, NRF_TIMER_EVENT_COMPARE0),
                                          nrf_drv_timer_task_address_get(&timer0, NRF_TIMER_TASK_START));
    APP_ERROR_CHECK(err_code);

    // Enable both configured PPI channels
    err_code = nrf_drv_ppi_channel_enable(ppi_channel1);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_ppi_channel_enable(ppi_channel2);
    APP_ERROR_CHECK(err_code);
}


/** @brief Function for Timer 0 initialization, which will be started and stopped by timer1 and timer2 using PPI.
*/
static void timer0_init(void)
{
    ret_code_t err_code = nrf_drv_timer_init(&timer0, NULL, timer_event_handler);
    APP_ERROR_CHECK(err_code);
}


/** @brief Function for Timer 1 initialization.
 *  @details Initializes Timer 1 peripheral, creates event and interrupt every 2 seconds,
 *           by configuring CC[0] to timer overflow value, we create events at even number of seconds
 *           for example, events are created at 2,4,6 ... seconds. This event can be used to stop Timer 0
 *           with Timer1->Event_Compare[0] triggering Timer 0 TASK_STOP through PPI.
*/
static void timer1_init(void)
{
    // Configure Timer 1 to overflow every 2 seconds. Check TIMER1 configuration for details
    // The overflow occurs every 0xFFFF/(SysClk/2^PRESCALER).
    // = 65535/31250 = 2.097 sec
    ret_code_t err_code = nrf_drv_timer_init(&timer1, NULL, timer_event_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_extended_compare(&timer1, NRF_TIMER_CC_CHANNEL0, 0xFFFFUL, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, false);
}


/** @brief Function for Timer 2 initialization.
 *  @details Initializes Timer 2 peripheral, creates event and interrupt every 2 seconds
 *           by configuring CC[0] to half of timer overflow value. Events are created at odd number of seconds.
 *           For example, events are created at 1,3,5,... seconds. This event can be used to start Timer 0
 *           with Timer2->Event_Compare[0] triggering Timer 0 TASK_START through PPI.
*/
static void timer2_init(void)
{
    // Generate interrupt/event when half of time before the timer overflows has past, that is at 1,3,5,7... seconds from start.
    // Check TIMER1 configuration for details
    // now the overflow occurs every 0xFFFF/(SysClk/2^PRESCALER)
    // = 65535/31250 = 2.097 sec */
    ret_code_t err_code = nrf_drv_timer_init(&timer2, NULL, timer_event_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_extended_compare(&timer2, NRF_TIMER_CC_CHANNEL0, 0x7FFFUL, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, false);
}


/**
 * @brief Function for application main entry.
 */
int main(void)
{
    timer0_init(); // Timer used to blink the LEDs.
    timer1_init(); // Timer to generate events on even number of seconds.
    timer2_init(); // Timer to generate events on odd number of seconds.
    ppi_init();    // PPI to redirect the event to timer start/stop tasks.

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

    // Enabling constant latency as indicated by PAN 11 "HFCLK: Base current with HFCLK 
    // running is too high" found at Product Anomaly document found at
    // https://www.nordicsemi.com/eng/Products/Bluetooth-R-low-energy/nRF51822/#Downloads
    //
    // @note This example does not go to low power mode therefore constant latency is not needed.
    //       However this setting will ensure correct behaviour when routing TIMER events through 
    //       PPI (shown in this example) and low power mode simultaneously.
    NRF_POWER->TASKS_CONSTLAT = 1;
    
    // Start clock.
    nrf_drv_timer_enable(&timer0);
    nrf_drv_timer_enable(&timer1);
    nrf_drv_timer_enable(&timer2);

    // Loop and increment the timer count value and capture value into LEDs. @note counter is only incremented between TASK_START and TASK_STOP.
    while (true)
    {

        printf("Current cout: %d\n\r", (int)nrf_drv_timer_capture(&timer0,NRF_TIMER_CC_CHANNEL0));

        /* increment the counter */
        nrf_drv_timer_increment(&timer0);

        nrf_delay_ms(100);
    }
}


/** @} */
