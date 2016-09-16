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
#include "sdk_config.h"
#if UART_ENABLED
#include "nrf_drv_uart.h"
#include "nrf_assert.h"
#include "nordic_common.h"
#include "nrf_drv_common.h"
#include "nrf_gpio.h"
#include "app_util_platform.h"

#if (defined(UARTE_IN_USE) && defined(UART_IN_USE))
    // UARTE and UART combined
    #define CODE_FOR_UARTE(code) if (m_cb[p_instance->drv_inst_idx].use_easy_dma) { code }
    #define CODE_FOR_UARTE_INT(idx, code) if (m_cb[idx].use_easy_dma) { code }
    #define CODE_FOR_UART(code)   else { code }
#elif (defined(UARTE_IN_USE) && !defined(UART_IN_USE))
    // UARTE only
    #define CODE_FOR_UARTE(code) { code }
    #define CODE_FOR_UARTE_INT(idx, code) { code }
    #define CODE_FOR_UART(code)
#elif (!defined(UARTE_IN_USE) && defined(UART_IN_USE))
    // UART only
    #define CODE_FOR_UARTE(code)
    #define CODE_FOR_UARTE_INT(idx, code)
    #define CODE_FOR_UART(code) { code }
#else
    #error "Wrong configuration."
#endif

#ifndef IS_EASY_DMA_RAM_ADDRESS
    #define IS_EASY_DMA_RAM_ADDRESS(addr) (((uint32_t)addr & 0xFFFF0000) == 0x20000000)
#endif

#define TX_COUNTER_ABORT_REQ_VALUE 256

typedef struct
{
    void                   * p_context;
    nrf_uart_event_handler_t handler;
    uint8_t          const * p_tx_buffer;
    uint8_t                * p_rx_buffer;
    uint8_t                * p_rx_secondary_buffer;
    volatile uint16_t        tx_counter;
    uint8_t                  tx_buffer_length;
    uint8_t                  rx_buffer_length;
    uint8_t                  rx_secondary_buffer_length;
    volatile uint8_t         rx_counter;
    bool                     rx_enabled;
    nrf_drv_state_t          state;
#if (defined(UARTE_IN_USE) && defined(UART_IN_USE))
    bool                     use_easy_dma;
#endif
} uart_control_block_t;

static uart_control_block_t m_cb[UART_COUNT];

__STATIC_INLINE void apply_config(nrf_drv_uart_t const * p_instance, nrf_drv_uart_config_t const * p_config)
{
    nrf_gpio_pin_set(p_config->pseltxd);
    nrf_gpio_cfg_output(p_config->pseltxd);
    nrf_gpio_cfg_input(p_config->pselrxd, NRF_GPIO_PIN_NOPULL);

    CODE_FOR_UARTE
    (
        nrf_uarte_baudrate_set(p_instance->reg.p_uarte, (nrf_uarte_baudrate_t)p_config->baudrate);
        nrf_uarte_configure(p_instance->reg.p_uarte, (nrf_uarte_parity_t)p_config->parity,
                            (nrf_uarte_hwfc_t)p_config->hwfc);
        nrf_uarte_txrx_pins_set(p_instance->reg.p_uarte, p_config->pseltxd, p_config->pselrxd);
        if (p_config->hwfc == NRF_UART_HWFC_ENABLED)
        {
            nrf_gpio_cfg_input(p_config->pselcts, NRF_GPIO_PIN_NOPULL);
            nrf_gpio_pin_set(p_config->pselrts);
            nrf_gpio_cfg_output(p_config->pselrts);
            nrf_uarte_hwfc_pins_set(p_instance->reg.p_uarte, p_config->pselrts, p_config->pselcts);
        }
    )
    CODE_FOR_UART
    (
        nrf_uart_baudrate_set(p_instance->reg.p_uart, p_config->baudrate);
        nrf_uart_configure(p_instance->reg.p_uart, p_config->parity, p_config->hwfc);
        nrf_uart_txrx_pins_set(p_instance->reg.p_uart, p_config->pseltxd, p_config->pselrxd);
        if (p_config->hwfc == NRF_UART_HWFC_ENABLED)
        {
            nrf_gpio_cfg_input(p_config->pselcts, NRF_GPIO_PIN_NOPULL);
            nrf_gpio_pin_set(p_config->pselrts);
            nrf_gpio_cfg_output(p_config->pselrts);
            nrf_uart_hwfc_pins_set(p_instance->reg.p_uart, p_config->pselrts, p_config->pselcts);
        }
    )
}

__STATIC_INLINE void interrupts_enable(const nrf_drv_uart_t * p_instance, uint8_t interrupt_priority)
{
    CODE_FOR_UARTE
    (
        nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_ENDRX);
        nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_ENDTX);
        nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_ERROR);
        nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_RXTO);
        nrf_uarte_int_enable(p_instance->reg.p_uarte, NRF_UARTE_INT_ENDRX_MASK |
                                         NRF_UARTE_INT_ENDTX_MASK |
                                         NRF_UARTE_INT_ERROR_MASK |
                                         NRF_UARTE_INT_RXTO_MASK);
    )
    CODE_FOR_UART
    (
        nrf_uart_event_clear(p_instance->reg.p_uart, NRF_UART_EVENT_TXDRDY);
        nrf_uart_event_clear(p_instance->reg.p_uart, NRF_UART_EVENT_RXTO);
        nrf_uart_int_enable(p_instance->reg.p_uart, NRF_UART_INT_MASK_TXDRDY |
                                       NRF_UART_INT_MASK_RXTO);
    )

    nrf_drv_common_irq_enable(nrf_drv_get_IRQn((void *)p_instance->reg.p_uart), interrupt_priority);
}

__STATIC_INLINE void interrupts_disable(const nrf_drv_uart_t * p_instance)
{
    CODE_FOR_UARTE
    (
        nrf_uarte_int_disable(p_instance->reg.p_uarte, NRF_UARTE_INT_ENDRX_MASK |
                                          NRF_UARTE_INT_ENDTX_MASK |
                                          NRF_UARTE_INT_ERROR_MASK |
                                          NRF_UARTE_INT_RXTO_MASK);
    )
    CODE_FOR_UART
    (
        nrf_uart_int_disable(p_instance->reg.p_uart, NRF_UART_INT_MASK_RXDRDY |
                                        NRF_UART_INT_MASK_TXDRDY |
                                        NRF_UART_INT_MASK_ERROR  |
                                        NRF_UART_INT_MASK_RXTO);
    )
    nrf_drv_common_irq_disable(nrf_drv_get_IRQn((void *)p_instance->reg.p_uart));
}

__STATIC_INLINE void pins_to_default(const nrf_drv_uart_t * p_instance)
{
    /* Reset pins to default states */
    uint32_t txd;
    uint32_t rxd;
    uint32_t rts;
    uint32_t cts;

    CODE_FOR_UARTE
    (
        txd = nrf_uarte_tx_pin_get(p_instance->reg.p_uarte);
        rxd = nrf_uarte_rx_pin_get(p_instance->reg.p_uarte);
        rts = nrf_uarte_rts_pin_get(p_instance->reg.p_uarte);
        cts = nrf_uarte_cts_pin_get(p_instance->reg.p_uarte);
        nrf_uarte_txrx_pins_disconnect(p_instance->reg.p_uarte);
        nrf_uarte_hwfc_pins_disconnect(p_instance->reg.p_uarte);
    )
    CODE_FOR_UART
    (
        txd = nrf_uart_tx_pin_get(p_instance->reg.p_uart);
        rxd = nrf_uart_rx_pin_get(p_instance->reg.p_uart);
        rts = nrf_uart_rts_pin_get(p_instance->reg.p_uart);
        cts = nrf_uart_cts_pin_get(p_instance->reg.p_uart);
        nrf_uart_txrx_pins_disconnect(p_instance->reg.p_uart);
        nrf_uart_hwfc_pins_disconnect(p_instance->reg.p_uart);
    )

    nrf_gpio_cfg_default(txd);
    nrf_gpio_cfg_default(rxd);

    if (cts != NRF_UART_PSEL_DISCONNECTED)
    {
        nrf_gpio_cfg_default(cts);
    }

    if (rts != NRF_UART_PSEL_DISCONNECTED)
    {
        nrf_gpio_cfg_default(rts);
    }

}

__STATIC_INLINE void uart_enable(const nrf_drv_uart_t * p_instance)
{
    CODE_FOR_UARTE(nrf_uarte_enable(p_instance->reg.p_uarte);)
    CODE_FOR_UART(nrf_uart_enable(p_instance->reg.p_uart););
}

__STATIC_INLINE void uart_disable(const nrf_drv_uart_t * p_instance)
{
    CODE_FOR_UARTE(nrf_uarte_disable(p_instance->reg.p_uarte);)
    CODE_FOR_UART(nrf_uart_disable(p_instance->reg.p_uart););
}

ret_code_t nrf_drv_uart_init(const nrf_drv_uart_t * p_instance, nrf_drv_uart_config_t const * p_config,
                             nrf_uart_event_handler_t event_handler)
{
    ASSERT(p_config);
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    if (p_cb->state != NRF_DRV_STATE_UNINITIALIZED)
    {
        return NRF_ERROR_INVALID_STATE;
    }

#if (defined(UARTE_IN_USE) && defined(UART_IN_USE))
    p_cb->use_easy_dma = p_config->use_easy_dma;
#endif
    apply_config(p_instance, p_config);

    p_cb->handler = event_handler;
    p_cb->p_context = p_config->p_context;

    if (p_cb->handler)
    {
        interrupts_enable(p_instance, p_config->interrupt_priority);
    }

    uart_enable(p_instance);
    p_cb->rx_buffer_length = 0;
    p_cb->rx_secondary_buffer_length = 0;
    p_cb->tx_buffer_length = 0;
    p_cb->state = NRF_DRV_STATE_INITIALIZED;
    p_cb->rx_enabled = false;
    return NRF_SUCCESS;
}

void nrf_drv_uart_uninit(const nrf_drv_uart_t * p_instance)
{
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    uart_disable(p_instance);

    if (p_cb->handler)
    {
        interrupts_disable(p_instance);
    }

    pins_to_default(p_instance);

    p_cb->state = NRF_DRV_STATE_UNINITIALIZED;
    p_cb->handler = NULL;
}

#if defined(UART_IN_USE)
__STATIC_INLINE void tx_byte(NRF_UART_Type * p_uart, uart_control_block_t * p_cb)
{
    nrf_uart_event_clear(p_uart, NRF_UART_EVENT_TXDRDY);
    uint8_t txd = p_cb->p_tx_buffer[p_cb->tx_counter];
    p_cb->tx_counter++;
    nrf_uart_txd_set(p_uart, txd);
}

__STATIC_INLINE ret_code_t nrf_drv_uart_tx_for_uart(const nrf_drv_uart_t * p_instance)
{
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    ret_code_t err_code = NRF_SUCCESS;

    nrf_uart_event_clear(p_instance->reg.p_uart, NRF_UART_EVENT_TXDRDY);
    nrf_uart_task_trigger(p_instance->reg.p_uart, NRF_UART_TASK_STARTTX);

    tx_byte(p_instance->reg.p_uart, p_cb);

    if (p_cb->handler == NULL)
    {
        while (p_cb->tx_counter < (uint16_t) p_cb->tx_buffer_length)
        {
            while (!nrf_uart_event_check(p_instance->reg.p_uart, NRF_UART_EVENT_TXDRDY) &&
                    p_cb->tx_counter != TX_COUNTER_ABORT_REQ_VALUE)
            {
            }
            if (p_cb->tx_counter != TX_COUNTER_ABORT_REQ_VALUE)
            {
                tx_byte(p_instance->reg.p_uart, p_cb);
            }
        }

        if (p_cb->tx_counter == TX_COUNTER_ABORT_REQ_VALUE)
        {
            err_code = NRF_ERROR_FORBIDDEN;
        }
        else
        {
            while (!nrf_uart_event_check(p_instance->reg.p_uart, NRF_UART_EVENT_TXDRDY))
            {
            }
            nrf_uart_task_trigger(p_instance->reg.p_uart, NRF_UART_TASK_STOPTX);
        }
        p_cb->tx_buffer_length = 0;
    }

    return err_code;
}
#endif

#if defined(UARTE_IN_USE)
__STATIC_INLINE ret_code_t nrf_drv_uart_tx_for_uarte(const nrf_drv_uart_t * p_instance)
{
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    ret_code_t err_code = NRF_SUCCESS;

    nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_ENDTX);
    nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_TXSTOPPED);
    nrf_uarte_tx_buffer_set(p_instance->reg.p_uarte, p_cb->p_tx_buffer, p_cb->tx_buffer_length);
    nrf_uarte_task_trigger(p_instance->reg.p_uarte, NRF_UARTE_TASK_STARTTX);

    if (p_cb->handler == NULL)
    {
        bool endtx;
        bool txstopped;
        do
        {
            endtx     = nrf_uarte_event_check(p_instance->reg.p_uarte, NRF_UARTE_EVENT_ENDTX);
            txstopped = nrf_uarte_event_check(p_instance->reg.p_uarte, NRF_UARTE_EVENT_TXSTOPPED);
        }
        while ((!endtx) && (!txstopped));

        if (txstopped)
        {
            err_code = NRF_ERROR_FORBIDDEN;
        }
        p_cb->tx_buffer_length = 0;
    }

    return err_code;
}
#endif

ret_code_t nrf_drv_uart_tx(const nrf_drv_uart_t * p_instance, uint8_t const * const p_data, uint8_t length)
{
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    ASSERT(p_cb->state == NRF_DRV_STATE_INITIALIZED);
    ASSERT(length>0);
    ASSERT(p_data);

    CODE_FOR_UARTE
    (
        // EasyDMA requires that transfer buffers are placed in DataRAM,
        // signal error if the are not.
        if (!IS_EASY_DMA_RAM_ADDRESS(p_data))
        {
            return NRF_ERROR_INVALID_ADDR;
        }
    )

    if (nrf_drv_uart_tx_in_progress(p_instance))
    {
        return NRF_ERROR_BUSY;
    }
    p_cb->tx_buffer_length = length;
    p_cb->p_tx_buffer      = p_data;
    p_cb->tx_counter       = 0;

    CODE_FOR_UARTE
    (
        return nrf_drv_uart_tx_for_uarte(p_instance);
    )
    CODE_FOR_UART
    (
        return nrf_drv_uart_tx_for_uart(p_instance);
    )
}

bool nrf_drv_uart_tx_in_progress(const nrf_drv_uart_t * p_instance)
{
    return (m_cb[p_instance->drv_inst_idx].tx_buffer_length != 0);
}

#if defined(UART_IN_USE)
__STATIC_INLINE void rx_enable(const nrf_drv_uart_t * p_instance)
{
    nrf_uart_event_clear(p_instance->reg.p_uart, NRF_UART_EVENT_ERROR);
    nrf_uart_event_clear(p_instance->reg.p_uart, NRF_UART_EVENT_RXDRDY);
    nrf_uart_task_trigger(p_instance->reg.p_uart, NRF_UART_TASK_STARTRX);
}

__STATIC_INLINE void rx_byte(NRF_UART_Type * p_uart, uart_control_block_t * p_cb)
{
    if (!p_cb->rx_buffer_length)
    {
        nrf_uart_event_clear(p_uart, NRF_UART_EVENT_RXDRDY);
        // Byte received when buffer is not set - data lost.
        (void) nrf_uart_rxd_get(p_uart);
        return;
    }
    nrf_uart_event_clear(p_uart, NRF_UART_EVENT_RXDRDY);
    p_cb->p_rx_buffer[p_cb->rx_counter] = nrf_uart_rxd_get(p_uart);
    p_cb->rx_counter++;
}

__STATIC_INLINE ret_code_t nrf_drv_uart_rx_for_uart(const nrf_drv_uart_t * p_instance, uint8_t * p_data, uint8_t length, bool second_buffer)
{
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    if ((!p_cb->rx_enabled) && (!second_buffer))
    {
        rx_enable(p_instance);
    }

    if (p_cb->handler == NULL)
    {
        nrf_uart_event_clear(p_instance->reg.p_uart, NRF_UART_EVENT_RXTO);

        bool rxrdy;
        bool rxto;
        bool error;
        do
        {
            do
            {
                error = nrf_uart_event_check(p_instance->reg.p_uart, NRF_UART_EVENT_ERROR);
                rxrdy = nrf_uart_event_check(p_instance->reg.p_uart, NRF_UART_EVENT_RXDRDY);
                rxto  = nrf_uart_event_check(p_instance->reg.p_uart, NRF_UART_EVENT_RXTO);
            } while ((!rxrdy) && (!rxto) && (!error));

            if (error || rxto)
            {
                break;
            }
            rx_byte(p_instance->reg.p_uart, p_cb);
        } while (p_cb->rx_buffer_length > p_cb->rx_counter);

        p_cb->rx_buffer_length = 0;
        if (error)
        {
            return NRF_ERROR_INTERNAL;
        }

        if (rxto)
        {
            return NRF_ERROR_FORBIDDEN;
        }

        if (p_cb->rx_enabled)
        {
            nrf_uart_task_trigger(p_instance->reg.p_uart, NRF_UART_TASK_STARTRX);
        }
        else
        {
            // Skip stopping RX if driver is forced to be enabled.
            nrf_uart_task_trigger(p_instance->reg.p_uart, NRF_UART_TASK_STOPRX);
        }
    }
    else
    {
        nrf_uart_int_enable(p_instance->reg.p_uart, NRF_UART_INT_MASK_RXDRDY | NRF_UART_INT_MASK_ERROR);
    }
    return NRF_SUCCESS;
}
#endif

#if defined(UARTE_IN_USE)
__STATIC_INLINE ret_code_t nrf_drv_uart_rx_for_uarte(const nrf_drv_uart_t * p_instance, uint8_t * p_data, uint8_t length, bool second_buffer)
{
    nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_ENDRX);
    nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_RXTO);
    nrf_uarte_rx_buffer_set(p_instance->reg.p_uarte, p_data, length);
    if (!second_buffer)
    {
        nrf_uarte_task_trigger(p_instance->reg.p_uarte, NRF_UARTE_TASK_STARTRX);
    }
    else
    {
        nrf_uarte_shorts_enable(p_instance->reg.p_uarte, NRF_UARTE_SHORT_ENDRX_STARTRX);
    }

    if (m_cb[p_instance->drv_inst_idx].handler == NULL)
    {
        bool endrx;
        bool rxto;
        bool error;
        do {
            endrx  = nrf_uarte_event_check(p_instance->reg.p_uarte, NRF_UARTE_EVENT_ENDRX);
            rxto   = nrf_uarte_event_check(p_instance->reg.p_uarte, NRF_UARTE_EVENT_RXTO);
            error  = nrf_uarte_event_check(p_instance->reg.p_uarte, NRF_UARTE_EVENT_ERROR);
        }while ((!endrx) && (!rxto) && (!error));

        m_cb[p_instance->drv_inst_idx].rx_buffer_length = 0;

        if (error)
        {
            return NRF_ERROR_INTERNAL;
        }

        if (rxto)
        {
            return NRF_ERROR_FORBIDDEN;
        }
    }
    else
    {
        nrf_uarte_int_enable(p_instance->reg.p_uarte, NRF_UARTE_INT_ERROR_MASK | NRF_UARTE_INT_ENDRX_MASK);
    }
    return NRF_SUCCESS;
}
#endif

ret_code_t nrf_drv_uart_rx(const nrf_drv_uart_t * p_instance, uint8_t * p_data, uint8_t length)
{
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    ASSERT(m_cb[p_instance->drv_inst_idx].state == NRF_DRV_STATE_INITIALIZED);
    ASSERT(length>0);

    CODE_FOR_UARTE
    (
        // EasyDMA requires that transfer buffers are placed in DataRAM,
        // signal error if the are not.
        if (!IS_EASY_DMA_RAM_ADDRESS(p_data))
        {
            return NRF_ERROR_INVALID_ADDR;
        }
    )

    bool second_buffer = false;

    if (p_cb->handler)
    {
        CODE_FOR_UARTE
        (
            nrf_uarte_int_disable(p_instance->reg.p_uarte, NRF_UARTE_INT_ERROR_MASK | NRF_UARTE_INT_ENDRX_MASK);
        )
        CODE_FOR_UART
        (
            nrf_uart_int_disable(p_instance->reg.p_uart, NRF_UART_INT_MASK_RXDRDY | NRF_UART_INT_MASK_ERROR);
        )
    }
    if (p_cb->rx_buffer_length != 0)
    {
        if (p_cb->rx_secondary_buffer_length != 0)
        {
            if (p_cb->handler)
            {
                CODE_FOR_UARTE
                (
                    nrf_uarte_int_enable(p_instance->reg.p_uarte, NRF_UARTE_INT_ERROR_MASK | NRF_UARTE_INT_ENDRX_MASK);
                )
                CODE_FOR_UART
                (
                    nrf_uart_int_enable(p_instance->reg.p_uart, NRF_UART_INT_MASK_RXDRDY | NRF_UART_INT_MASK_ERROR);
                )
            }
            return NRF_ERROR_BUSY;
        }
        second_buffer = true;
    }

    if (!second_buffer)
    {
        p_cb->rx_buffer_length = length;
        p_cb->p_rx_buffer      = p_data;
        p_cb->rx_counter       = 0;
        p_cb->rx_secondary_buffer_length = 0;
    }
    else
    {
        p_cb->p_rx_secondary_buffer = p_data;
        p_cb->rx_secondary_buffer_length = length;
    }

    CODE_FOR_UARTE
    (
        return nrf_drv_uart_rx_for_uarte(p_instance, p_data, length, second_buffer);
    )
    CODE_FOR_UART
    (
        return nrf_drv_uart_rx_for_uart(p_instance, p_data, length, second_buffer);
    )
}

void nrf_drv_uart_rx_enable(const nrf_drv_uart_t * p_instance)
{
    //Easy dma mode does not support enabling receiver without setting up buffer.
    CODE_FOR_UARTE
    (
        ASSERT(false);
    )
    CODE_FOR_UART
    (
        if (!m_cb[p_instance->drv_inst_idx].rx_enabled)
        {
            rx_enable(p_instance);
            m_cb[p_instance->drv_inst_idx].rx_enabled = true;
        }
    )
}

void nrf_drv_uart_rx_disable(const nrf_drv_uart_t * p_instance)
{
    //Easy dma mode does not support enabling receiver without setting up buffer.
    CODE_FOR_UARTE
    (
        ASSERT(false);
    )
    CODE_FOR_UART
    (
        nrf_uart_task_trigger(p_instance->reg.p_uart, NRF_UART_TASK_STOPRX);
        m_cb[p_instance->drv_inst_idx].rx_enabled = false;
    )
}

uint32_t nrf_drv_uart_errorsrc_get(const nrf_drv_uart_t * p_instance)
{
    uint32_t errsrc;
    CODE_FOR_UARTE
    (
        nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_ERROR);
        errsrc = nrf_uarte_errorsrc_get_and_clear(p_instance->reg.p_uarte);
    )
    CODE_FOR_UART
    (
        nrf_uart_event_clear(p_instance->reg.p_uart, NRF_UART_EVENT_ERROR);
        errsrc = nrf_uart_errorsrc_get_and_clear(p_instance->reg.p_uart);
    )
    return errsrc;
}

__STATIC_INLINE void rx_done_event(uart_control_block_t * p_cb, uint8_t bytes, uint8_t * p_data)
{
    nrf_drv_uart_event_t event;

    event.type             = NRF_DRV_UART_EVT_RX_DONE;
    event.data.rxtx.bytes  = bytes;
    event.data.rxtx.p_data = p_data;

    p_cb->handler(&event, p_cb->p_context);
}

__STATIC_INLINE void tx_done_event(uart_control_block_t * p_cb, uint8_t bytes)
{
    nrf_drv_uart_event_t event;

    event.type             = NRF_DRV_UART_EVT_TX_DONE;
    event.data.rxtx.bytes  = bytes;
    event.data.rxtx.p_data = (uint8_t *)p_cb->p_tx_buffer;

    p_cb->tx_buffer_length = 0;

    p_cb->handler(&event, p_cb->p_context);
}

void nrf_drv_uart_tx_abort(const nrf_drv_uart_t * p_instance)
{
    uart_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    CODE_FOR_UARTE
    (
        nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_TXSTOPPED);
        nrf_uarte_task_trigger(p_instance->reg.p_uarte, NRF_UARTE_TASK_STOPTX);
        if (p_cb->handler == NULL)
        {
            while (!nrf_uarte_event_check(p_instance->reg.p_uarte, NRF_UARTE_EVENT_TXSTOPPED));
        }
    )
    CODE_FOR_UART
    (
        nrf_uart_task_trigger(p_instance->reg.p_uart, NRF_UART_TASK_STOPTX);
        if (p_cb->handler)
        {
            tx_done_event(p_cb, p_cb->tx_counter);
        }
        else
        {
            p_cb->tx_counter       = TX_COUNTER_ABORT_REQ_VALUE;
        }
    )
}

void nrf_drv_uart_rx_abort(const nrf_drv_uart_t * p_instance)
{
    CODE_FOR_UARTE
    (
        nrf_uarte_task_trigger(p_instance->reg.p_uarte, NRF_UARTE_TASK_STOPRX);
    )
    CODE_FOR_UART
    (
        nrf_uart_int_disable(p_instance->reg.p_uart, NRF_UART_INT_MASK_RXDRDY | NRF_UART_INT_MASK_ERROR);
        nrf_uart_task_trigger(p_instance->reg.p_uart, NRF_UART_TASK_STOPRX);
    )
}


#if defined(UART_IN_USE)
__STATIC_INLINE void uart_irq_handler(NRF_UART_Type * p_uart, uart_control_block_t * p_cb)
{
    if (nrf_uart_int_enable_check(p_uart, NRF_UART_INT_MASK_ERROR) &&
        nrf_uart_event_check(p_uart, NRF_UART_EVENT_ERROR))
    {
        nrf_drv_uart_event_t event;
        nrf_uart_event_clear(p_uart, NRF_UART_EVENT_ERROR);
        nrf_uart_int_disable(p_uart, NRF_UART_INT_MASK_RXDRDY | NRF_UART_INT_MASK_ERROR);
        if (!p_cb->rx_enabled)
        {
            nrf_uart_task_trigger(p_uart, NRF_UART_TASK_STOPRX);
        }
        event.type                   = NRF_DRV_UART_EVT_ERROR;
        event.data.error.error_mask  = nrf_uart_errorsrc_get_and_clear(p_uart);
        event.data.error.rxtx.bytes  = p_cb->rx_buffer_length;
        event.data.error.rxtx.p_data = p_cb->p_rx_buffer;

        //abort transfer
        p_cb->rx_buffer_length = 0;
        p_cb->rx_secondary_buffer_length = 0;

        p_cb->handler(&event,p_cb->p_context);
    }
    else if (nrf_uart_int_enable_check(p_uart, NRF_UART_INT_MASK_RXDRDY) &&
             nrf_uart_event_check(p_uart, NRF_UART_EVENT_RXDRDY))
    {
        rx_byte(p_uart, p_cb);
        if (p_cb->rx_buffer_length == p_cb->rx_counter)
        {
            if (p_cb->rx_secondary_buffer_length)
            {
                uint8_t * p_data     = p_cb->p_rx_buffer;
                uint8_t   rx_counter = p_cb->rx_counter;

                //Switch to secondary buffer.
                p_cb->rx_buffer_length = p_cb->rx_secondary_buffer_length;
                p_cb->p_rx_buffer = p_cb->p_rx_secondary_buffer;
                p_cb->rx_secondary_buffer_length = 0;
                p_cb->rx_counter = 0;
                rx_done_event(p_cb, rx_counter, p_data);
            }
            else
            {
                if (!p_cb->rx_enabled)
                {
                    nrf_uart_task_trigger(p_uart, NRF_UART_TASK_STOPRX);
                }
                nrf_uart_int_disable(p_uart, NRF_UART_INT_MASK_RXDRDY | NRF_UART_INT_MASK_ERROR);
                p_cb->rx_buffer_length = 0;
                rx_done_event(p_cb, p_cb->rx_counter, p_cb->p_rx_buffer);
            }
        }
    }

    if (nrf_uart_event_check(p_uart, NRF_UART_EVENT_TXDRDY))
    {
        if (p_cb->tx_counter < (uint16_t) p_cb->tx_buffer_length)
        {
            tx_byte(p_uart, p_cb);
        }
        else
        {
            nrf_uart_event_clear(p_uart, NRF_UART_EVENT_TXDRDY);
            if (p_cb->tx_buffer_length)
            {
                tx_done_event(p_cb, p_cb->tx_buffer_length);
            }
        }
    }

    if (nrf_uart_event_check(p_uart, NRF_UART_EVENT_RXTO))
    {
        nrf_uart_event_clear(p_uart, NRF_UART_EVENT_RXTO);

        // RXTO event may be triggered as a result of abort call. In th
        if (p_cb->rx_enabled)
        {
            nrf_uart_task_trigger(p_uart, NRF_UART_TASK_STARTRX);
        }
        if (p_cb->rx_buffer_length)
        {
            p_cb->rx_buffer_length = 0;
            rx_done_event(p_cb, p_cb->rx_counter, p_cb->p_rx_buffer);
        }
    }
}
#endif

#if defined(UARTE_IN_USE)
__STATIC_INLINE void uarte_irq_handler(NRF_UARTE_Type * p_uarte, uart_control_block_t * p_cb)
{
    if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_ERROR))
    {
        nrf_drv_uart_event_t event;

        nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_ERROR);

        event.type                   = NRF_DRV_UART_EVT_ERROR;
        event.data.error.error_mask  = nrf_uarte_errorsrc_get_and_clear(p_uarte);
        event.data.error.rxtx.bytes  = nrf_uarte_rx_amount_get(p_uarte);
        event.data.error.rxtx.p_data = p_cb->p_rx_buffer;

        //abort transfer
        p_cb->rx_buffer_length = 0;
        p_cb->rx_secondary_buffer_length = 0;

        p_cb->handler(&event, p_cb->p_context);
    }
    else if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_ENDRX))
    {
        nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_ENDRX);
        uint8_t amount = nrf_uarte_rx_amount_get(p_uarte);
        // If the transfer was stopped before completion, amount of transfered bytes
        // will not be equal to the buffer length. Interrupted trunsfer is ignored.
        if (amount == p_cb->rx_buffer_length)
        {
            if (p_cb->rx_secondary_buffer_length)
            {
                uint8_t * p_data = p_cb->p_rx_buffer;
                nrf_uarte_shorts_disable(p_uarte, NRF_UARTE_SHORT_ENDRX_STARTRX);
                p_cb->rx_buffer_length = p_cb->rx_secondary_buffer_length;
                p_cb->p_rx_buffer = p_cb->p_rx_secondary_buffer;
                p_cb->rx_secondary_buffer_length = 0;
                rx_done_event(p_cb, amount, p_data);
            }
            else
            {
                p_cb->rx_buffer_length = 0;
                rx_done_event(p_cb, amount, p_cb->p_rx_buffer);
            }
        }
    }

    if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_RXTO))
    {
        nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_RXTO);
        if (p_cb->rx_buffer_length)
        {
            p_cb->rx_buffer_length = 0;
            rx_done_event(p_cb, nrf_uarte_rx_amount_get(p_uarte), p_cb->p_rx_buffer);
        }
    }

    if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_ENDTX))
    {
        nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_ENDTX);
        if (p_cb->tx_buffer_length)
        {
            tx_done_event(p_cb, nrf_uarte_tx_amount_get(p_uarte));
        }
    }
}
#endif

void UART0_IRQHandler(void)
{
    CODE_FOR_UARTE_INT
    (
        UART0_INSTANCE_INDEX,
        uarte_irq_handler(NRF_UARTE0, &m_cb[UART0_INSTANCE_INDEX]);
    )
    CODE_FOR_UART
    (
        uart_irq_handler(NRF_UART0, &m_cb[UART0_INSTANCE_INDEX]);
    )
}
#endif //UART_ENABLED
