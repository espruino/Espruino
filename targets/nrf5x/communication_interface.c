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
#include "jspininfo.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf.h"

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

unsigned int getNRFBaud(int baud) {
  switch (baud) {
    case 1200: return UART_BAUDRATE_BAUDRATE_Baud1200;
    case 2400: return UART_BAUDRATE_BAUDRATE_Baud2400;
    case 4800: return UART_BAUDRATE_BAUDRATE_Baud4800;
    case 9600: return UART_BAUDRATE_BAUDRATE_Baud9600;
    case 14400: return UART_BAUDRATE_BAUDRATE_Baud14400;
    case 19200: return UART_BAUDRATE_BAUDRATE_Baud19200;
    case 28800: return UART_BAUDRATE_BAUDRATE_Baud28800;
    case 38400: return UART_BAUDRATE_BAUDRATE_Baud38400;
    case 57600: return UART_BAUDRATE_BAUDRATE_Baud57600;
    case 76800: return UART_BAUDRATE_BAUDRATE_Baud76800;
    case 115200: return UART_BAUDRATE_BAUDRATE_Baud115200;
    case 230400: return UART_BAUDRATE_BAUDRATE_Baud230400;
    case 250000: return UART_BAUDRATE_BAUDRATE_Baud250000;
    case 460800: return UART_BAUDRATE_BAUDRATE_Baud460800;
    case 921600: return UART_BAUDRATE_BAUDRATE_Baud921600;
    case 1000000: return UART_BAUDRATE_BAUDRATE_Baud1M;
    default: return 0;
  }
}

int uart_init()
{
  uint32_t err_code;

  const app_uart_comm_params_t comm_params =
  {
      pinInfo[DEFAULT_CONSOLE_RX_PIN].pin,
      pinInfo[DEFAULT_CONSOLE_TX_PIN].pin,
      UART_PIN_DISCONNECTED,
      UART_PIN_DISCONNECTED,
      APP_UART_FLOW_CONTROL_DISABLED,
      false,
      getNRFBaud(DEFAULT_CONSOLE_BAUDRATE)
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
