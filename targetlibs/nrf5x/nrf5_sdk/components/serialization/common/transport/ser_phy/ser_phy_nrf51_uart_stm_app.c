/* Copyright (c) Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of other
 * contributors to this software may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * 4. This software must only be used in a processor manufactured by Nordic
 * Semiconductor ASA, or in a processor manufactured by STMicroelectronics that
 * is used in combination with a processor manufactured by Nordic Semiconductor
 * or in a processor manufactured by STMicroelectronics.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdint.h>
#include <stdlib.h>

#include "nordic_common.h"
#include "boards.h"
#include "nrf.h"
#include "nrf_error.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_gpiote.h"
#include "app_gpiote.h"
#include "app_util.h"
#include "app_util_platform.h"

#include "ser_phy.h"
#include "ser_phy_config_app_nrf51.h"

#define UART_PIN_DISCONNECTED 0xFFFFFFFF /**< Value indicating that no pin is connected to this UART register. */

/** @brief States for the app_uart state machine. */
typedef enum
{
    UART_IDLE,              /**< Indicating that the current status for either RX or TX is idle. When both RX and TX is idle the UART will be disabled in order to save power. */
    UART_RX,                /**< Used to indicate that a packet is currently being received on RX. */
    UART_RX_PENDING,        /**< Used to indicate that byte is ready at RXD register but no buffer was available when the byte was received. The byte will be pulled when a buffer is set. */
    UART_TX_COMPLETE,       /**< Used on TX to indicate that final byte has been put on TXD register. Next TXDRDY interrupt will indicate that last byte has been transmitted. */
    UART_TX_SEND,           /**< Used to indicate that a packet is currently being transmitted on TX. */
    UART_TX_LAST_BYTE_WAIT, /**< Used to indicate that the last byte on a TX packet is currently waiting to be transmitted when CTS goes low. Note that flow control is off when tranmitting final byte. */
    UART_STALL,             /**< Indicates that TX is stalled because final byte is being received on the UART. */
} uart_states_t;

static volatile uint32_t m_pin_cts_mask; /**< CTS pin mask for UART module. */

static volatile uint8_t * mp_tx_stream;                          /**< Pointer to array of data packet to be transmitted. */
static volatile uint16_t  m_tx_stream_length;                    /**< Total length of data packet to be transmitted. */
static volatile uint16_t  m_tx_stream_index;                     /**< Index in data packet for next byte to be transmitted. */
static          uint8_t   m_tx_length_buf[SER_PHY_HEADER_SIZE];  /**< Buffer needed in transmission of packet length */

static          uint8_t * mp_rx_stream;                          /**< Pointer to current receive buffer. */
static volatile uint16_t  m_rx_stream_length;                    /**< Length of receive buffer. */
static volatile uint16_t  m_rx_stream_index;                     /**< Index in receive buffer where the next byte will be placed. */
static volatile bool      m_rx_stream_header;                    /**< Indication of whether header data (true) or payload data (false) is currently being received. */
static          uint8_t   m_rx_length_buf[SER_PHY_HEADER_SIZE];  /**< Buffer needed in reception of packet length */
static          uint8_t   m_rx_drop_buf[1];                      /**< Additional buffer, needed by packet dropping functionality. */

static volatile uart_states_t m_rx_state            = UART_IDLE; /**< State of the RX state machine. */
static volatile uart_states_t m_tx_state            = UART_IDLE; /**< State of the TX state machine. */
static volatile bool          m_tx_pending          = false;     /**< If TX state is UART_STALL and a byte is ready for tranmission the pending flag is set to true. */
static volatile bool          m_cts_high_disconnect = false;     /**< If CTS was sampled low when last byte was transmitted this flag is set to true to indicate that a switch from low->high on CTS should be interpreted as transmission has completed and UART is to be disabled to save power. */

static volatile ser_phy_events_handler_t m_ser_phy_event_handler;
static volatile ser_phy_evt_t            m_ser_phy_event_rx;
static volatile ser_phy_evt_t            m_ser_phy_event_tx;


static void uart_peripheral_disconnect_flow(void)
{
    NRF_GPIO->OUTSET = 1 << SER_PHY_UART_RTS;
    nrf_gpio_cfg_output(SER_PHY_UART_RTS);

    NRF_UART0->PSELCTS = UART_PIN_DISCONNECTED;
    NRF_UART0->PSELRTS = UART_PIN_DISCONNECTED;
    NRF_UART0->CONFIG &= ~(UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
}


static void uart_peripheral_connect_flow(void)
{
    NRF_UART0->PSELCTS = SER_PHY_UART_CTS;
    NRF_UART0->PSELRTS = SER_PHY_UART_RTS;
    NRF_UART0->CONFIG |= (UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
}


static void uart_peripheral_enable(void)
{
    if (!(NRF_UART0->ENABLE & (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos)))
    {
        NRF_UART0->PSELCTS       = SER_PHY_UART_CTS;
        NRF_UART0->PSELRTS       = SER_PHY_UART_RTS;
        NRF_UART0->CONFIG       |= (UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
        NRF_UART0->TASKS_STARTRX = 1;
        NRF_UART0->TASKS_STARTTX = 1;
        NRF_UART0->ENABLE        = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
    }
}


static void uart_peripheral_disable(void)
{
    if ((m_tx_state == UART_IDLE || m_tx_state == UART_STALL) &&
        (m_rx_state == UART_IDLE))
    {
        NRF_UART0->TASKS_STOPTX = 1;
        NRF_UART0->TASKS_STOPRX = 1;
        NRF_UART0->PSELCTS      = UART_PIN_DISCONNECTED;
        NRF_UART0->PSELRTS      = UART_PIN_DISCONNECTED;
        NRF_UART0->ENABLE       = (UART_ENABLE_ENABLE_Disabled << UART_ENABLE_ENABLE_Pos);

        nrf_gpio_cfg_input(SER_PHY_UART_RTS, NRF_GPIO_PIN_NOPULL);

        nrf_gpiote_event_configure(0, SER_PHY_UART_CTS, NRF_GPIOTE_POLARITY_TOGGLE);

        if (!nrf_gpio_pin_read(SER_PHY_UART_CTS))
        {
            NRF_GPIO->OUTSET = 1 << SER_PHY_UART_RTS;
            nrf_gpio_cfg_output(SER_PHY_UART_RTS);
            uart_peripheral_enable();
        }
    }
}


static void uart_tx_start(void)
{
    if (mp_tx_stream != NULL)
    {
        //If RX is already ongoing then no wakeup signal is required.
        if (m_rx_state == UART_IDLE)
        {
            nrf_gpiote_event_disable(0);
            NRF_GPIO->OUTSET = 1 << SER_PHY_UART_RTS;
            nrf_gpio_cfg_output(SER_PHY_UART_RTS);
            uart_peripheral_connect_flow();
            uart_peripheral_enable();
        }
    }
}


static void uart_tx_send(void)
{
    //First send 2 bytes of header then payload
    if (m_tx_stream_index < SER_PHY_HEADER_SIZE)
    {
        NRF_UART0->TXD = m_tx_length_buf[m_tx_stream_index++];
    }
    else if (m_tx_stream_index < m_tx_stream_length)
    {
        NRF_UART0->TXD = mp_tx_stream[m_tx_stream_index++ - SER_PHY_HEADER_SIZE];
    }
}


static void uart_tx_last_byte(void)
{
    uart_peripheral_disconnect_flow();
    m_tx_state = UART_TX_LAST_BYTE_WAIT;

    //Configure event in case CTS is going low during this function execution
    nrf_gpiote_event_configure(0, SER_PHY_UART_CTS, NRF_GPIOTE_POLARITY_TOGGLE);

    if (!nrf_gpio_pin_read(SER_PHY_UART_CTS)) //All pins are low --> last byte can be transmitted.
    {
        //Re-check state as it might have changed due to preemption of current interrupt.
        nrf_gpiote_event_disable(0);

        if (m_tx_state == UART_TX_LAST_BYTE_WAIT)
        {
            m_tx_state = UART_TX_COMPLETE;
            uart_tx_send();
        }
    }
}


static void tx_complete_event_send(void)
{
    mp_tx_stream       = NULL;
    m_tx_stream_length = 0;
    m_tx_stream_index  = 0;

    m_ser_phy_event_tx.evt_type = SER_PHY_EVT_TX_PKT_SENT;
    m_ser_phy_event_handler(m_ser_phy_event_tx);
}


static void uart_txdrdy_handle(void)
{
    NRF_UART0->EVENTS_TXDRDY = 0;

    if (m_tx_state == UART_TX_SEND || m_tx_state == UART_IDLE)
    {
        if (m_tx_stream_index < (m_tx_stream_length - 1))
        {
            m_tx_state = UART_TX_SEND;
            uart_tx_send();
            //Keep same state.
        }
        else if (m_tx_stream_index == m_tx_stream_length)
        {
            m_tx_state = UART_IDLE;
            tx_complete_event_send();
        }
        else
        {
            uart_tx_last_byte();
        }
    }
    else if (m_tx_state == UART_TX_COMPLETE)
    {
        m_tx_state = UART_IDLE;

        if (m_rx_state == UART_IDLE)
        {
            nrf_delay_us(15);
            nrf_gpiote_event_configure(0, SER_PHY_UART_CTS, NRF_GPIOTE_POLARITY_TOGGLE);

            if (nrf_gpio_pin_read(SER_PHY_UART_CTS))
            {
                uart_peripheral_disable();
            }
            else
            {
                uart_peripheral_connect_flow();
                m_cts_high_disconnect = true;
                nrf_gpiote_event_configure(0, SER_PHY_UART_CTS, NRF_GPIOTE_POLARITY_TOGGLE);

                if (nrf_gpio_pin_read(SER_PHY_UART_CTS))
                {
                    //If second sample show CTS high it either
                    //1) happened BEFORE gpiote enable and uart should be disabled.
                    //(m_cts_high_disconnect == true).
                    //2) happened AFTER gpiote enable and an interrupt low->high has occured then
                    //uart should NOT be disabled as the ISR has disabled the UART.
                    //(m_cts_high_disconnect == false).
                    if (m_cts_high_disconnect == true)
                    {
                        m_cts_high_disconnect = false;
                        uart_peripheral_disable();
                    }
                }
            }
        }
        else
        {
            uart_peripheral_connect_flow();
        }

        tx_complete_event_send();
    }
    else if (m_tx_state == UART_STALL)
    {
        if (m_tx_stream_index == m_tx_stream_length)
        {
            tx_complete_event_send();
        }
        else
        {
            m_tx_pending = true;
        }
    }
    else
    {
        //Do nothing.
    }
}


static __INLINE void on_cts_high(void)
{
    if (m_cts_high_disconnect == true)
    {
        m_cts_high_disconnect = false;

        if (m_rx_state == UART_IDLE && m_tx_state == UART_IDLE)
        {
            if (m_tx_stream_index == m_tx_stream_length)
            {
                uart_peripheral_disable();
            }
        }
    }
}


static __INLINE void on_cts_low(void)
{
    m_cts_high_disconnect = false;
    nrf_gpiote_event_disable(0);

    if (m_tx_state == UART_STALL)
    {
        m_tx_pending = true;
    }
    else if (m_tx_state == UART_TX_LAST_BYTE_WAIT)
    {
        m_tx_state = UART_TX_COMPLETE;
        uart_tx_send();
    }
    else if (m_rx_state == UART_IDLE && m_tx_state == UART_IDLE)
    {
        NRF_GPIO->OUTSET = 1 << SER_PHY_UART_RTS;
        nrf_gpio_cfg_output(SER_PHY_UART_RTS);
        uart_peripheral_enable();
    }
}


static void uart_rxdrdy_handle(void)
{
    if (m_rx_state == UART_IDLE)
    {
        m_rx_state = UART_RX;
    }

    //Set proper size and buff at the beginning of receiving header
    if ((m_rx_stream_header == true) && !m_rx_stream_index)
    {
        m_rx_stream_length = SER_PHY_HEADER_SIZE;
        mp_rx_stream       = m_rx_length_buf;
    }

    if (mp_rx_stream != NULL)
    {
        bool tx_dual_end = false;

        NRF_UART0->EVENTS_RXDRDY = 0;

        //Second last byte received.
        //Disconnect CTS before pulling the byte and receiving the final byte.
        if ((m_rx_stream_header == false) && ((m_rx_stream_index) == (m_rx_stream_length - 2)))
        {
            nrf_gpio_cfg_output(SER_PHY_UART_RTS);

            //Last byte is waiting for tansmission. Thus dual end TX case.
            //
            if (m_tx_state == UART_TX_LAST_BYTE_WAIT)
            {
                m_tx_state = UART_STALL;
                nrf_gpiote_event_disable(0);

                //Checking pending state.
                //- If pending is true then CTS have become low after we stalled the UART and final byte should be transmitted here.
                //- If pending is false we should check if final byte was tranmitted and if not, the do the transmission her.
                if ((m_tx_pending == true) || (m_tx_stream_index == (m_tx_stream_length - 1)))
                {
                    //tx_dual_end = true;
                    NRF_GPIO->OUTSET = 1 << SER_PHY_UART_RTS;
                    uart_tx_send();
                }
            }

            if (!tx_dual_end)
            {
                NRF_GPIO->OUTCLR = 1 << SER_PHY_UART_RTS;
            }
            m_tx_state = UART_STALL;

            NRF_UART0->PSELCTS = UART_PIN_DISCONNECTED;
            NRF_UART0->PSELRTS = UART_PIN_DISCONNECTED;
            NRF_UART0->CONFIG &= ~(UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
        }

        if (m_rx_stream_index < (m_rx_stream_length - 1))
        {
            if (mp_rx_stream != m_rx_drop_buf)
            {
                mp_rx_stream[m_rx_stream_index++] = NRF_UART0->RXD;
            }
            else
            {
                mp_rx_stream[0] = NRF_UART0->RXD;
                m_rx_stream_index++;
            }

            if (m_tx_stream_index == (m_tx_stream_length - 1))
            {
                //Toggle CTS line to indicate ack.
                //If CTS is connected to UART this code will have no effect.
                //But on edge case on bi-directional last byte transfer this avoids lock-up.
                NRF_GPIO->OUTSET = 1 << SER_PHY_UART_RTS;
                nrf_delay_us(8);
                NRF_GPIO->OUTCLR = 1 << SER_PHY_UART_RTS;
            }
        }
        else
        {
            if (m_rx_stream_header == false)
            {
                NRF_GPIO->OUTSET = 1 << SER_PHY_UART_RTS;

                if (mp_rx_stream != m_rx_drop_buf)
                {
                    mp_rx_stream[m_rx_stream_index++] = NRF_UART0->RXD;
                }
                else
                {
                    mp_rx_stream[0] = NRF_UART0->RXD;
                    m_rx_stream_index++;
                }
                m_rx_state = UART_IDLE;

                //Last byte of payload received - notify that next transmission will be header
                m_rx_stream_header = true;

                //Prepare event
                if (mp_rx_stream != m_rx_drop_buf)
                {
                    m_ser_phy_event_rx.evt_type = SER_PHY_EVT_RX_PKT_RECEIVED;
                    m_ser_phy_event_rx.evt_params.rx_pkt_received.num_of_bytes = m_rx_stream_index;
                    m_ser_phy_event_rx.evt_params.rx_pkt_received.p_buffer     = mp_rx_stream;
                }
                else
                {
                    m_ser_phy_event_rx.evt_type = SER_PHY_EVT_RX_PKT_DROPPED;
                }

                m_rx_stream_length = 0;
                m_rx_stream_index  = 0;
            }
            else
            {
                mp_rx_stream[m_rx_stream_index++] = NRF_UART0->RXD;

                //Last byte of header received - notify that next transmission will be payload
                m_rx_stream_header = false;

                mp_rx_stream = NULL;

                //Clear index before receiving payload
                m_rx_stream_index = 0;

                //Prepare event
                m_rx_stream_length = uint16_decode(m_rx_length_buf);
                m_ser_phy_event_rx.evt_type = SER_PHY_EVT_RX_BUF_REQUEST;
                m_ser_phy_event_rx.evt_params.rx_buf_request.num_of_bytes = m_rx_stream_length;
            }

            //Notify upwards
            m_ser_phy_event_handler(m_ser_phy_event_rx);

            //UART TX was stalled while receiving final byte. Restart tx.
            if (m_tx_state == UART_STALL)
            {
                if (m_tx_stream_length == m_tx_stream_index)
                {
                    m_tx_state = UART_IDLE;
                }
                else if (m_tx_stream_index == (m_tx_stream_length - 1))
                {
                    m_tx_state = UART_TX_LAST_BYTE_WAIT;
                }
                else
                {
                    m_tx_state = UART_TX_SEND;
                }

                //Critical region for avoiding timing issues in 'simultaneous RX end and TX start'
                CRITICAL_REGION_ENTER();
                if (m_tx_pending == true)
                {
                    m_tx_pending = false;
                    uart_tx_start();

                    if (m_tx_state == UART_TX_SEND)
                    {
                        uart_tx_send();
                    }
                    else if (m_tx_state == UART_TX_LAST_BYTE_WAIT)
                    {
                        uart_tx_last_byte();
                    }
                }
                CRITICAL_REGION_EXIT();

                if (m_tx_state == UART_IDLE)
                {
                    uart_peripheral_disable();
                }
            }
        }
    }
    else
    {
        m_rx_state = UART_RX_PENDING;
    }
}


static __INLINE void uart_error_handle(void)
{
    uint32_t error_source;

    //Clear UART ERROR event flag.
    NRF_UART0->EVENTS_ERROR = 0;

    //Clear error source.
    error_source        = NRF_UART0->ERRORSRC;
    NRF_UART0->ERRORSRC = error_source;

    m_ser_phy_event_rx.evt_type                       = SER_PHY_EVT_HW_ERROR;
    m_ser_phy_event_rx.evt_params.hw_error.error_code = error_source;

    m_ser_phy_event_handler(m_ser_phy_event_rx);
}


/**@brief app_gpiote_fast_detect event handler.
 *
 */
static void gpiote_evt_handler(void)
{
    if ( m_pin_cts_mask)
    {
        if ( nrf_gpio_pin_read(SER_PHY_UART_CTS) )
        {
            on_cts_high();
        }
        else
        {
            on_cts_low();
        }
    }
}

/**@brief Function for handling the UART Interrupt.
 *
 * @details UART interrupt handler to process TX Ready when TXD is available, RX Ready when a byte
 *          is received, or in case of error when receiving a byte.
 */
void UART0_IRQHandler(void)
{
    //Handle Reception.
    if (NRF_UART0->EVENTS_RXDRDY != 0 && (NRF_UART0->INTENSET & UART_INTENSET_RXDRDY_Msk))
    {
        uart_rxdrdy_handle();
    }

    //Handle transmission.
    if (NRF_UART0->EVENTS_TXDRDY != 0 && (NRF_UART0->INTENSET & UART_INTENSET_TXDRDY_Msk))
    {
        uart_txdrdy_handle();
    }

    //Handle errors.
    if (NRF_UART0->EVENTS_ERROR != 0 && (NRF_UART0->INTENSET & UART_INTENSET_ERROR_Msk))
    {
        uart_error_handle();
    }
}


uint32_t ser_phy_open(ser_phy_events_handler_t events_handler)
{
    if (events_handler == NULL)
    {
        return NRF_ERROR_NULL;
    }

    //Check if function was not called before
    if (m_ser_phy_event_handler != NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    //GPIO Setup
    nrf_gpio_cfg_input(SER_PHY_UART_RTS, NRF_GPIO_PIN_NOPULL);

    NRF_GPIO->OUTSET = 1 << SER_PHY_UART_TX;
    nrf_gpio_cfg_output(SER_PHY_UART_TX);

    //Setup the gpiote to handle pin events on cts-pin.
    //For the UART we want to detect both low->high and high->low transitions in order to
    //know when to activate/de-activate the TX/RX in the UART.
    //Configure pin.
    m_pin_cts_mask = (1 << SER_PHY_UART_CTS);
    nrf_gpio_cfg_sense_input(SER_PHY_UART_CTS,
                             NRF_GPIO_PIN_PULLUP,
                             NRF_GPIO_PIN_SENSE_LOW);

    nrf_gpio_cfg_sense_input(SER_PHY_UART_RX,
                             NRF_GPIO_PIN_PULLUP,
                             NRF_GPIO_PIN_NOSENSE);

    (void)app_gpiote_input_event_handler_register(0,
                                                  SER_PHY_UART_CTS,
                                                  GPIOTE_CONFIG_POLARITY_Toggle,
                                                  gpiote_evt_handler);
    (void)app_gpiote_enable_interrupts();

    NVIC_ClearPendingIRQ(PendSV_IRQn);

    m_rx_state = UART_IDLE;
    m_tx_state = UART_IDLE;

    //Set header flag
    m_rx_stream_header = true;

    //UART setup
    NRF_UART0->PSELRXD  = SER_PHY_UART_RX;
    NRF_UART0->PSELTXD  = SER_PHY_UART_TX;
    NRF_UART0->PSELCTS  = UART_PIN_DISCONNECTED;
    NRF_UART0->PSELRTS  = UART_PIN_DISCONNECTED;
    NRF_UART0->BAUDRATE = SER_PHY_UART_BAUDRATE;
    NRF_UART0->CONFIG   = (UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
    NRF_UART0->ENABLE   = (UART_ENABLE_ENABLE_Disabled << UART_ENABLE_ENABLE_Pos);

    //Enable UART interrupt
    NRF_UART0->INTENCLR = 0xFFFFFFFF;
    NRF_UART0->INTENSET = (UART_INTENSET_TXDRDY_Set << UART_INTENSET_TXDRDY_Pos) |
                          (UART_INTENSET_RXDRDY_Set << UART_INTENSET_RXDRDY_Pos) |
                          (UART_INTENSET_ERROR_Set << UART_INTENSET_ERROR_Pos);

    NVIC_ClearPendingIRQ(UART0_IRQn);
    NVIC_SetPriority(UART0_IRQn, APP_IRQ_PRIORITY_MID);
    NVIC_EnableIRQ(UART0_IRQn);

    m_ser_phy_event_handler = events_handler;

    return NRF_SUCCESS;
}


void ser_phy_close(void)
{
    //Disable UART interrupt.
    NRF_UART0->INTENCLR = 0xFFFFFFFF;

    //Unregister callback.
    m_ser_phy_event_handler = NULL;

    //Will not check err_code here as we will still continue with closure of UART despite errors.
    //Note that any error will still be reported back in the system.
    nrf_gpiote_event_disable(0);

    uart_peripheral_disable();

    //Clear internal UART states
    m_rx_state = UART_IDLE;
    m_tx_state = UART_IDLE;

    mp_tx_stream       = NULL;
    m_tx_stream_length = 0;
    m_tx_stream_index  = 0;

    mp_rx_stream       = NULL;
    m_rx_stream_length = 0;
    m_rx_stream_index  = 0;
}


uint32_t ser_phy_tx_pkt_send(const uint8_t * p_buffer, uint16_t num_of_bytes)
{
    if (p_buffer == NULL)
    {
        return NRF_ERROR_NULL;
    }

    if (num_of_bytes == 0)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    if (mp_tx_stream != NULL)
    {
        return NRF_ERROR_BUSY;
    }

    m_tx_pending = true;

    (void) uint16_encode(num_of_bytes, m_tx_length_buf);
    mp_tx_stream       = (uint8_t *)p_buffer;
    m_tx_stream_length = num_of_bytes + SER_PHY_HEADER_SIZE;

    //Critical region for avoiding timing issues in 'simultaneous RX end and TX start'
    CRITICAL_REGION_ENTER();
    if ((!m_rx_stream_length) || (m_rx_stream_index < (m_rx_stream_length - 2)))
    {
        if (m_tx_state != UART_STALL)
        {
            if (m_tx_pending == true)
            {
                m_tx_pending = false;
                uart_tx_start();
                //As no tx can be ongoing, then it is safe to call tx_send here.
                uart_tx_send();
            }
        }
    }
    CRITICAL_REGION_EXIT();

    return NRF_SUCCESS;
}


uint32_t ser_phy_rx_buf_set(uint8_t * p_buffer)
{
    if (m_ser_phy_event_rx.evt_type != SER_PHY_EVT_RX_BUF_REQUEST)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (p_buffer != NULL)
    {
        mp_rx_stream = p_buffer;
    }
    else
    {
        mp_rx_stream = m_rx_drop_buf;
    }

    return NRF_SUCCESS;
}


void ser_phy_interrupts_enable(void)
{
    NVIC_EnableIRQ(UART0_IRQn);
    NVIC_EnableIRQ(GPIOTE_IRQn);
}


void ser_phy_interrupts_disable(void)
{
    NVIC_DisableIRQ(UART0_IRQn);
    NVIC_DisableIRQ(GPIOTE_IRQn);
}
