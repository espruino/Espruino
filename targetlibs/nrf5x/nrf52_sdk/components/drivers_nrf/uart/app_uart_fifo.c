/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
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
#include "app_fifo.h"
#include "nrf.h"
#include "nrf_drv_gpiote.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "app_util.h"

static __INLINE uint32_t fifo_length(app_fifo_t * const fifo)
{
  uint32_t tmp = fifo->read_pos;
  return fifo->write_pos - tmp;
}

#define FIFO_LENGTH(F) fifo_length(&F)              /**< Macro to calculate length of a FIFO. */


#define UART_INSTANCE_GPIOTE_BASE  0x00FF                                   /**< Define the base for UART instance ID when flow control is used. The userid from GPIOTE will be used with padded 0xFF at LSB for easy converting the instance id to GPIOTE id. */
#define UART_INSTANCE_ID_INVALID   0x0000                                   /**< Value 0x0000 is used to indicate an invalid instance id. When 0 is provided as instance id upon initialization, the module will provide a valid id to the caller. */

/** @brief States for the app_uart state machine. */
typedef enum
{
    UART_OFF,                                                               /**< app_uart state OFF, indicating CTS is low. */
    UART_READY,                                                             /**< app_uart state ON, indicating CTS is high. */
    UART_ON,                                                                /**< app_uart state TX, indicating UART is ongoing transmitting data. */
    UART_WAIT_CLOSE,                                                        /**< app_uart state WAIT CLOSE, indicating that CTS is low, but a byte is currently being transmitted on the line. */
} app_uart_states_t;

/** @brief State transition events for the app_uart state machine. */
typedef enum
{
    ON_CTS_HIGH,                                                            /**< Event: CTS gone high. */
    ON_CTS_LOW,                                                             /**< Event: CTS gone low. */
    ON_UART_PUT,                                                            /**< Event: Application wants to transmit data. */
    ON_TX_READY,                                                            /**< Event: Data has been transmitted on the uart and line is available. */
    ON_UART_CLOSE,                                                          /**< Event: The UART module are being stopped. */
} app_uart_state_event_t;

static app_fifo_t                  m_rx_fifo;                               /**< RX FIFO buffer for storing data received on the UART until the application fetches them using app_uart_get(). */
static app_fifo_t                  m_tx_fifo;                               /**< TX FIFO buffer for storing data to be transmitted on the UART when TXD is ready. Data is put to the buffer on using app_uart_put(). */

static uint8_t                     m_instance_counter = 1;                        /**< CTS pin mask for UART module. */
static app_uart_event_handler_t    m_event_handler;                         /**< Event handler function. */
static volatile app_uart_states_t  m_current_state = UART_OFF;              /**< State of the state machine. */

/**@brief Function for disabling the UART when entering the UART_OFF state.
 */
static void action_uart_deactivate(void)
{
    m_current_state         = UART_OFF;
    NRF_UART0->TASKS_STOPTX = 1;
    NRF_UART0->TASKS_STOPRX = 1;
    NRF_UART0->ENABLE       = (UART_ENABLE_ENABLE_Disabled << UART_ENABLE_ENABLE_Pos);
}


void action_tx_stop()
{
    app_uart_evt_t app_uart_event;
    // No more bytes in FIFO, terminate transmission.
    NRF_UART0->TASKS_STOPTX = 1;
    m_current_state         = UART_READY;
    // Last byte from FIFO transmitted, notify the application.
    // Notify that new data is available if this was first byte put in the buffer.
    app_uart_event.evt_type = APP_UART_TX_EMPTY;
    m_event_handler(&app_uart_event);
}


/**@brief Function for sending the next byte in the TX buffer. Called when (re-)entering the UART_ON state.
 *       If no more data is available in the TX buffer, the state machine will enter UART_READY state.
 */
static void action_tx_send()
{
    uint8_t tx_byte;

    if (m_current_state != UART_ON)
    {
        // Start the UART.
        NRF_UART0->TASKS_STARTTX = 1;
    }

    if (app_fifo_get(&m_tx_fifo, &tx_byte) != NRF_SUCCESS)
    {
        action_tx_stop();
        return;
    }

    NRF_UART0->INTENCLR = (UART_INTENSET_TXDRDY_Set << UART_INTENSET_TXDRDY_Pos);
    NRF_UART0->TXD      = tx_byte;
    m_current_state     = UART_ON;
    NRF_UART0->INTENSET = (UART_INTENSET_TXDRDY_Set << UART_INTENSET_TXDRDY_Pos);
}


static void action_tx_ready()
{
    // Get next byte from FIFO.
    if (FIFO_LENGTH(m_tx_fifo) != 0)
    {
        action_tx_send();
    }
    else
    {
        action_tx_stop();
    }
}


/**@brief Function for the handling of the ON_CTS_HIGH event.
 */
static void on_cts_high(void)
{
    switch (m_current_state)
    {
        case UART_READY:
            action_uart_deactivate();
            break;

        case UART_ON:
            m_current_state = UART_WAIT_CLOSE;
            break;

        default:
            // Nothing to do.
            break;
    }
}


/**@brief Function for the handling of the ON_CTS_LOW event.
 */
static void on_cts_low(void)
{
    switch (m_current_state)
    {
        case UART_OFF:
            NRF_UART0->ENABLE        = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
            NRF_UART0->TASKS_STARTRX = 1;

            if (FIFO_LENGTH(m_tx_fifo) != 0)
            {
                action_tx_send();
            }
            else
            {
                m_current_state = UART_READY;
            }
            break;

        case UART_WAIT_CLOSE:
            m_current_state = UART_ON;
            break;

        default:
            // Nothing to do.
            break;
    }
}


/**@brief Function for the handling of the ON_TX_READY event.
 */
static void on_tx_ready(void)
{
    switch (m_current_state)
    {
        case UART_WAIT_CLOSE:
            action_uart_deactivate();
            break;

        case UART_ON:
        case UART_READY:
            action_tx_ready();
            break;

        default:
            // Nothing to do.
            break;
    }
}


/**@brief Function for the handling of the ON_UART_PUT event when application has put data in the TX buffer.
 */
static void on_uart_put(void)
{
    if (m_current_state == UART_READY)
    {
        action_tx_send();
    }
}


/**@brief Function for the handling of the ON_UART_CLOSE event when application is closing the UART module.
 */
static void on_uart_close(void)
{
    action_uart_deactivate();
}


/**@brief Function for the state machine main event handler.
 *
 * @param[in]  event    Event that has occurred.
 */
static void on_uart_event(app_uart_state_event_t event)
{
    switch (event)
    {
        case ON_CTS_HIGH:
            on_cts_high();
            break;

        case ON_CTS_LOW:
            on_cts_low();
            break;

        case ON_TX_READY:
            on_tx_ready();
            break;

        case ON_UART_PUT:
            on_uart_put();
            break;

        case ON_UART_CLOSE:
            on_uart_close();
            break;

        default:
            // All valid events are handled above.
            break;
    }
}


/**@brief Function for the GPIOTE event handler.
 */
static void gpiote_uart_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if (nrf_drv_gpiote_in_is_set(pin))
    {
        on_uart_event(ON_CTS_HIGH);
    }
    else
    {
        on_uart_event(ON_CTS_LOW);
    }
}


/**@brief Function for the UART Interrupt handler.
 *
 * @details UART interrupt handler to process TX Ready when TXD is available, RX Ready when a byte
 *          is received, or in case of error when receiving a byte.
 */
void UART0_IRQHandler(void)
{
    // Handle reception
    if (NRF_UART0->EVENTS_RXDRDY != 0)
    {
        uint32_t err_code;

        // Clear UART RX event flag
        NRF_UART0->EVENTS_RXDRDY = 0;

        // Write received byte to FIFO
        err_code = app_fifo_put(&m_rx_fifo, (uint8_t)NRF_UART0->RXD);
        if (err_code != NRF_SUCCESS)
        {
            app_uart_evt_t app_uart_event;
            app_uart_event.evt_type          = APP_UART_FIFO_ERROR;
            app_uart_event.data.error_code   = err_code;
            m_event_handler(&app_uart_event);
        }
        // Notify that new data is available if this was first byte put in the buffer.
        else if (FIFO_LENGTH(m_rx_fifo) == 1)
        {
            app_uart_evt_t app_uart_event;
            app_uart_event.evt_type = APP_UART_DATA_READY;
            m_event_handler(&app_uart_event);
        }
        else
        {
            // Do nothing, only send event if first byte was added or overflow in FIFO occurred.
        }
    }

    // Handle transmission.
    if (NRF_UART0->EVENTS_TXDRDY != 0)
    {
        // Clear UART TX event flag.
        NRF_UART0->EVENTS_TXDRDY = 0;
        on_uart_event(ON_TX_READY);
    }

    // Handle errors.
    if (NRF_UART0->EVENTS_ERROR != 0)
    {
        uint32_t       error_source;
        app_uart_evt_t app_uart_event;

        // Clear UART ERROR event flag.
        NRF_UART0->EVENTS_ERROR = 0;

        // Clear error source.
        error_source        = NRF_UART0->ERRORSRC;
        NRF_UART0->ERRORSRC = error_source;

        app_uart_event.evt_type                 = APP_UART_COMMUNICATION_ERROR;
        app_uart_event.data.error_communication = error_source;

        m_event_handler(&app_uart_event);
    }
}


/**@brief Function for initialization of UART when flow control is disabled.
 */
static void uart_no_flow_control_init(void)
{
    NRF_UART0->ENABLE        = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
    NRF_UART0->EVENTS_RXDRDY = 0;
    NRF_UART0->EVENTS_TXDRDY = 0;

    NRF_UART0->CONFIG       &= ~(UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);

    NRF_UART0->PSELRTS       = UART_PIN_DISCONNECTED;
    NRF_UART0->PSELCTS       = UART_PIN_DISCONNECTED;

    NRF_UART0->TASKS_STARTTX = 1;
    NRF_UART0->TASKS_STARTRX = 1;
}


/**@brief Function for initialization of UART when standard flow control is enabled.
 */
static void uart_standard_flow_control_init(const app_uart_comm_params_t * p_comm_params)
{
    NRF_UART0->ENABLE        = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
    NRF_UART0->EVENTS_RXDRDY = 0;
    NRF_UART0->EVENTS_TXDRDY = 0;

    NRF_UART0->CONFIG       |= (UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);

    NRF_UART0->PSELCTS       = p_comm_params->cts_pin_no;
    NRF_UART0->PSELRTS       = p_comm_params->rts_pin_no;

    NRF_UART0->TASKS_STARTTX = 1;
    NRF_UART0->TASKS_STARTRX = 1;
}


uint32_t app_uart_init(const app_uart_comm_params_t * p_comm_params,
                             app_uart_buffers_t *     p_buffers,
                             app_uart_event_handler_t event_handler,
                             app_irq_priority_t       irq_priority,
                             uint16_t *               p_app_uart_uid)
{
    uint32_t err_code;

    m_current_state = UART_OFF;
    m_event_handler = event_handler;

    if (p_buffers == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    // Configure buffer RX buffer.
    err_code = app_fifo_init(&m_rx_fifo, p_buffers->rx_buf, p_buffers->rx_buf_size);
    if (err_code != NRF_SUCCESS)
    {
        // Propagate error code.
        return err_code;
    }

    // Configure buffer TX buffer.
    err_code = app_fifo_init(&m_tx_fifo, p_buffers->tx_buf, p_buffers->tx_buf_size);
    if (err_code != NRF_SUCCESS)
    {
        // Propagate error code.
        return err_code;
    }

    // Configure RX and TX pins.
    nrf_gpio_cfg_output(p_comm_params->tx_pin_no);
    nrf_gpio_cfg_input(p_comm_params->rx_pin_no, NRF_GPIO_PIN_NOPULL);

    NRF_UART0->PSELTXD = p_comm_params->tx_pin_no;
    NRF_UART0->PSELRXD = p_comm_params->rx_pin_no;

    // Configure baud rate and parity.
    NRF_UART0->BAUDRATE = (p_comm_params->baud_rate << UART_BAUDRATE_BAUDRATE_Pos);
    if (p_comm_params->use_parity)
    {
        NRF_UART0->CONFIG = (UART_CONFIG_PARITY_Included << UART_CONFIG_PARITY_Pos);
    }
    else
    {
        NRF_UART0->CONFIG = (UART_CONFIG_PARITY_Excluded << UART_CONFIG_PARITY_Pos);
    }

    if (p_comm_params->flow_control == APP_UART_FLOW_CONTROL_LOW_POWER)
    {
        if (!nrf_drv_gpiote_is_init())
        {
            err_code = nrf_drv_gpiote_init();
            if (err_code != NRF_SUCCESS)
            {
                return err_code;
            }
        }

        // Configure hardware flow control.
        nrf_drv_gpiote_out_config_t rts_config = GPIOTE_CONFIG_OUT_SIMPLE(true);
        err_code = nrf_drv_gpiote_out_init(p_comm_params->rts_pin_no, &rts_config);
        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }

        NRF_UART0->PSELCTS = UART_PIN_DISCONNECTED;
        NRF_UART0->PSELRTS = p_comm_params->rts_pin_no;
        NRF_UART0->CONFIG |= (UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);

        // Setup the gpiote to handle pin events on cts-pin.
        // For the UART we want to detect both low->high and high->low transitions in order to
        // know when to activate/de-activate the TX/RX in the UART.
        // Configure pin.
        nrf_drv_gpiote_in_config_t cts_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
        err_code = nrf_drv_gpiote_in_init(p_comm_params->cts_pin_no, &cts_config, gpiote_uart_event_handler);
        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }

        nrf_drv_gpiote_in_event_enable(p_comm_params->cts_pin_no, true);

        // UART CTS pin is active when low.
        if (nrf_drv_gpiote_in_is_set(p_comm_params->cts_pin_no))
        {
            on_uart_event(ON_CTS_HIGH);
        }
        else
        {
            on_uart_event(ON_CTS_LOW);
        }

    }
    else if (p_comm_params->flow_control == APP_UART_FLOW_CONTROL_ENABLED)
    {
        uart_standard_flow_control_init(p_comm_params);
        m_current_state = UART_READY;
    }
    else
    {
        uart_no_flow_control_init();
        m_current_state = UART_READY;
    }

    if (*p_app_uart_uid == UART_INSTANCE_ID_INVALID)
    {
        *p_app_uart_uid = m_instance_counter++;
    }

    // Enable UART interrupt
    NRF_UART0->INTENCLR = 0xffffffffUL;
    NRF_UART0->INTENSET = (UART_INTENSET_RXDRDY_Set << UART_INTENSET_RXDRDY_Pos) |
                          (UART_INTENSET_TXDRDY_Set << UART_INTENSET_TXDRDY_Pos) |
                          (UART_INTENSET_ERROR_Set << UART_INTENSET_ERROR_Pos);

    NVIC_ClearPendingIRQ(UART0_IRQn);
    NVIC_SetPriority(UART0_IRQn, irq_priority);
    NVIC_EnableIRQ(UART0_IRQn);

    return NRF_SUCCESS;
}


uint32_t app_uart_get(uint8_t * p_byte)
{
    return app_fifo_get(&m_rx_fifo, p_byte);
}


uint32_t app_uart_put(uint8_t byte)
{
    uint32_t err_code;

    err_code = app_fifo_put(&m_tx_fifo, byte);

    on_uart_event(ON_UART_PUT);

    return err_code;
}


uint32_t app_uart_flush(void)
{
    uint32_t err_code;

    err_code = app_fifo_flush(&m_rx_fifo);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = app_fifo_flush(&m_tx_fifo);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}


uint32_t app_uart_get_connection_state(app_uart_connection_state_t * p_conn_state)
{
    *p_conn_state = ((m_current_state == UART_OFF) ? APP_UART_DISCONNECTED : APP_UART_CONNECTED);

    return NRF_SUCCESS;
}

uint32_t app_uart_close(uint16_t app_uart_uid)
{
    if (app_uart_uid < UART_INSTANCE_GPIOTE_BASE)
    {
        on_uart_event(ON_UART_CLOSE);
        return NRF_SUCCESS;
    }

    on_uart_event(ON_UART_CLOSE);

    nrf_drv_gpiote_in_uninit(NRF_UART0->PSELCTS);
    return NRF_SUCCESS;
}

