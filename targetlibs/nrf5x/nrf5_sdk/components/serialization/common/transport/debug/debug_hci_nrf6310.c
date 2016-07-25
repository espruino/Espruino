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

#include <stddef.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"

#ifdef SER_PHY_HCI_DEBUG_ENABLE

#include "ser_phy_debug_comm.h"
#include "debug_hci_config_nRF6310.h"
#include "app_error.h"

#define LED_BLINK_us                  1               /* 1 us */

static hci_dbg_event_handler_t m_hci_dbg_event_handler = NULL;

static void pio_blink(uint8_t led)
{
    nrf_gpio_pin_set(led);
    nrf_delay_us(LED_BLINK_us);
    nrf_gpio_pin_clear(led);
}


static void Debug_PulseOnMemCallback()
{
    pio_blink(LED_MEM_CALLBACK);
}


static void Debug_PulseOnRxCallback()
{
    pio_blink(LED_RX_CALLBACK);
}


static void Debug_PulseOnTxCallback()
{
    pio_blink(LED_TX_CALLBACK);
}


static void Debug_PulseOnPacketDroppedCallback()
{
    pio_blink(LED_DP_CALLBACK);
}

static void Debug_PulseOnTxError()
{
    pio_blink(LED_TX_ERR_CALLBACK);
}

static void Debug_PulseOnPacketTX()
{
    pio_blink(PIO_SLIP_EVT_PKT_TX);
}

static void Debug_PulseOnAckTX()
{
    pio_blink(PIO_SLIP_EVT_ACK_TX);
}

static void Debug_PulseOnPacketTXED()
{
    pio_blink(PIO_SLIP_EVT_PKT_TXED);
}

static void Debug_PulseOnAckTXED()
{
    pio_blink(PIO_SLIP_EVT_ACK_TXED);
}

static void Debug_PulseOnPacketRXED()
{
    pio_blink(PIO_SLIP_EVT_PKT_RXED);
}

static void Debug_PulseOnAckRXED()
{
    pio_blink(PIO_SLIP_EVT_ACK_RXED);
}

static void Debug_PulseOnErrRXED()
{
    pio_blink(PIO_SLIP_EVT_ERR_RXED);
    /* throw assert when in debug mode*/
    APP_ERROR_CHECK_BOOL(false);
}

static void Debug_PulseOnTimer()
{
    pio_blink(PIO_TIMER_EVT_TIMEOUT);
}

static void Debug_PulseOnRETX()
{
    pio_blink(PIO_HCI_RETX);
}

static void Debug_LevelOnBusy(uint32_t data)
{
    if (data)
    {
      nrf_gpio_pin_set(PIO_MAIN_BUSY);
    }
    else
    {
      nrf_gpio_pin_clear(PIO_MAIN_BUSY);
    }
}

static void Debug_PulseOnTXReq()
{
    pio_blink(PIO_TX_REQ);
}

static void default_hci_event_handler(hci_dbg_evt_t event)
{
    switch (event.evt)
    {
    case HCI_PHY_EVT_TX_PKT_SENT:
      Debug_PulseOnTxCallback();
    break;
    case HCI_PHY_EVT_BUF_REQUEST:
      Debug_PulseOnMemCallback();
    break;
    case HCI_PHY_EVT_RX_PKT_RECEIVED:
      Debug_PulseOnRxCallback();
    break;
    case HCI_PHY_EVT_RX_PKT_DROPPED:
      Debug_PulseOnPacketDroppedCallback();
    break;
    case HCI_PHY_EVT_TX_ERROR:
      Debug_PulseOnTxError();
    break;
    case HCI_SLIP_EVT_PACKET_TX:
      Debug_PulseOnPacketTX();
    break;
    case HCI_SLIP_EVT_ACK_TX:
      Debug_PulseOnAckTX();
    break;
    case HCI_SLIP_EVT_PACKET_TXED:
      Debug_PulseOnPacketTXED();
    break;
    case HCI_SLIP_EVT_ACK_TXED:
      Debug_PulseOnAckTXED();
    break;
    case HCI_SLIP_EVT_PACKET_RXED:
      Debug_PulseOnPacketRXED();
    break;
    case HCI_SLIP_EVT_ACK_RXED:
      Debug_PulseOnAckRXED();
    break;
    case HCI_SLIP_EVT_ERR_RXED:
      Debug_PulseOnErrRXED();
    break;
    case HCI_TIMER_EVT_TIMEOUT:
      Debug_PulseOnTimer();
    break;
    case HCI_RETX:
      Debug_PulseOnRETX();
    break;
    case HCI_MAIN_BUSY:
      Debug_LevelOnBusy(event.data);
    break;
    case HCI_TX_REQ:
      Debug_PulseOnTXReq();
    break;

    default:
    break;
    }
    return;
}


void debug_init(hci_dbg_event_handler_t evt_callback)
{
    //Configure all LED as outputs.
    nrf_gpio_range_cfg_output(LED_START, LED_STOP);

    nrf_gpio_cfg_output(PIO_SLIP_EVT_PKT_TX);
    nrf_gpio_cfg_output(PIO_SLIP_EVT_ACK_TX);
    nrf_gpio_cfg_output(PIO_SLIP_EVT_PKT_TXED);
    nrf_gpio_cfg_output(PIO_SLIP_EVT_ACK_TXED);
    nrf_gpio_cfg_output(PIO_SLIP_EVT_PKT_RXED);
    nrf_gpio_cfg_output(PIO_SLIP_EVT_ACK_RXED);
    nrf_gpio_cfg_output(PIO_TIMER_EVT_TIMEOUT);
    nrf_gpio_cfg_output(PIO_HCI_RETX);
    nrf_gpio_cfg_output(PIO_MAIN_BUSY);
    nrf_gpio_cfg_output(PIO_TX_REQ);
    m_hci_dbg_event_handler = evt_callback;
    if (evt_callback)
    {
      m_hci_dbg_event_handler = evt_callback;
    }
    else
    {
      m_hci_dbg_event_handler = default_hci_event_handler;
    }

}


void debug_evt(hci_dbg_evt_type_t evt, uint32_t data)
{
    hci_dbg_evt_t event;
    event.evt = evt;
    event.data = data;
    
    if (m_hci_dbg_event_handler)
    {
      m_hci_dbg_event_handler(event);
    }
}

#endif
