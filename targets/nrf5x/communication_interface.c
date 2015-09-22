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

#include "communication_interface.h"

#include "jsdevices.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf.h"
#include "bsp.h"

// UART callback function. Registered in uart_init(). Allows to asychronously read characters from UART.
void uart_event_handle(app_uart_evt_t * p_event)
{
  
  if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
  {
    APP_ERROR_HANDLER(p_event->data.error_communication);
  }
  else if (p_event->evt_type == APP_UART_FIFO_ERROR)
  {
    APP_ERROR_HANDLER(p_event->data.error_code);
  }
  else if (p_event->evt_type == APP_UART_DATA_READY) // There is one byte in the RX FIFO.
  {
    uint8_t character;
    while(app_uart_get(&character) != NRF_SUCCESS); // Non blocking.
    jshPushIOCharEvent(EV_SERIAL1, (char) character);
  }

}

// Initialzes non blocking serial communication with terminal via uart. Returns 0 on success, -1 on an error.
int uart_init()
{
  uint32_t err_code;

  const app_uart_comm_params_t comm_params =
  {
      RX_PIN_NUMBER,
      TX_PIN_NUMBER,
      UART_PIN_DISCONNECTED,
      UART_PIN_DISCONNECTED,
      APP_UART_FLOW_CONTROL_DISABLED,
      false,
      UART_BAUDRATE_BAUDRATE_Baud38400
  };

  APP_UART_FIFO_INIT(&comm_params,
                     UART_RX_BUF_SIZE,
                     UART_TX_BUF_SIZE,
                     uart_event_handle,
                     APP_IRQ_PRIORITY_LOW,
                     err_code);

  APP_ERROR_CHECK(err_code);

  if (err_code != NRF_SUCCESS)
  {
    return -1;
  }
  return 0;
}
