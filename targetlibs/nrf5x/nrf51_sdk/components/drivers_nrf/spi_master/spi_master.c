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

#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf_soc.h"
#include "nrf51_bitfields.h"
#include "spi_master.h"

#if defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE)

typedef struct
{
    NRF_SPI_Type * p_nrf_spi;   /**< A pointer to the NRF SPI master */
    IRQn_Type irq_type;         /**< A type of NVIC IRQn */

    uint8_t * p_tx_buffer;      /**< A pointer to TX buffer. */
    uint16_t tx_length;         /**< A length of TX buffer. */
    uint16_t tx_index;          /**< A index of the current element in the TX buffer. */

    uint8_t * p_rx_buffer;      /**< A pointer to RX buffer. */
    uint16_t rx_length;         /**< A length RX buffer. */
    uint16_t rx_index;          /**< A index of the current element in the RX buffer. */

    uint16_t max_length;        /**< Max length (Max of the TX and RX length). */
    uint16_t bytes_count;
    uint8_t pin_slave_select;   /**< A pin for Slave Select. */

    spi_master_event_handler_t callback_event_handler;  /**< A handler for event callback function. */

    spi_master_state_t state;   /**< A state of an instance of SPI master. */
    bool started_flag;
    bool disable_all_irq;

} spi_master_instance_t;

#define _static static

_static volatile spi_master_instance_t m_spi_master_instances[SPI_MASTER_HW_ENABLED_COUNT];

/* Function prototypes */
static __INLINE volatile spi_master_instance_t * spi_master_get_instance(
    const spi_master_hw_instance_t spi_master_hw_instance);
static __INLINE void spi_master_send_recv_irq(volatile spi_master_instance_t * const p_spi_instance);

#endif //SPI_MASTER_0_ENABLE || SPI_MASTER_1_ENABLE

#ifdef SPI_MASTER_0_ENABLE
/**
 * @brief SPI0 interrupt handler.
 */
void SPI0_TWI0_IRQHandler(void)
{
    if ((NRF_SPI0->EVENTS_READY == 1) && (NRF_SPI0->INTENSET & SPI_INTENSET_READY_Msk))
    {
        NRF_SPI0->EVENTS_READY = 0;

        volatile spi_master_instance_t * p_spi_instance = spi_master_get_instance(SPI_MASTER_0);

        spi_master_send_recv_irq(p_spi_instance);
    }
}
#endif //SPI_MASTER_0_ENABLE

#ifdef SPI_MASTER_1_ENABLE
/**
 * @brief SPI0 interrupt handler.
 */
void SPI1_TWI1_IRQHandler(void)
{
    if ((NRF_SPI1->EVENTS_READY == 1) && (NRF_SPI1->INTENSET & SPI_INTENSET_READY_Msk))
    {
        NRF_SPI1->EVENTS_READY = 0;

        volatile spi_master_instance_t * p_spi_instance = spi_master_get_instance(SPI_MASTER_1);

        spi_master_send_recv_irq(p_spi_instance);
    }
}
#endif //SPI_MASTER_1_ENABLE

#if defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE)
/**
 * @breif Function for getting an instance of SPI master.
 */
static __INLINE volatile spi_master_instance_t * spi_master_get_instance(
    const spi_master_hw_instance_t spi_master_hw_instance)
{
    if (spi_master_hw_instance < SPI_MASTER_HW_ENABLED_COUNT)
    {
        return &(m_spi_master_instances[(uint8_t)spi_master_hw_instance]);
    }
    return NULL;
}

/**
 * @brief Function for initializing instance of SPI master by default values.
 */
static __INLINE void spi_master_init_hw_instance(NRF_SPI_Type *                   p_nrf_spi,
                                                 IRQn_Type                        irq_type,
                                                 volatile spi_master_instance_t * p_spi_instance,
                                                 volatile bool                    disable_all_irq)
{
    APP_ERROR_CHECK_BOOL(p_spi_instance != NULL);

    p_spi_instance->p_nrf_spi = p_nrf_spi;
    p_spi_instance->irq_type  = irq_type;

    p_spi_instance->p_tx_buffer = NULL;
    p_spi_instance->tx_length   = 0;
    p_spi_instance->tx_index    = 0;

    p_spi_instance->p_rx_buffer = NULL;
    p_spi_instance->rx_length   = 0;
    p_spi_instance->rx_index    = 0;

    p_spi_instance->bytes_count      = 0;
    p_spi_instance->max_length       = 0;
    p_spi_instance->pin_slave_select = 0;

    p_spi_instance->callback_event_handler = NULL;

    p_spi_instance->state           = SPI_MASTER_STATE_DISABLED;
    p_spi_instance->started_flag    = false;
    p_spi_instance->disable_all_irq = disable_all_irq;
}

/**
 * @brief Function for initializing TX or RX buffer.
 */
static __INLINE void spi_master_buffer_init(uint8_t * const           p_buf,
                                            const uint16_t            buf_len,
                                            uint8_t * volatile *      pp_buf,
                                            volatile uint16_t * const p_buf_len,
                                            volatile uint16_t * const p_index)
{
    APP_ERROR_CHECK_BOOL(pp_buf != NULL);
    APP_ERROR_CHECK_BOOL(p_buf_len != NULL);
    APP_ERROR_CHECK_BOOL(p_index != NULL);

    *pp_buf    = p_buf;
    *p_buf_len = (p_buf != NULL) ? buf_len : 0;
    *p_index   = 0;
}

/**
 * @brief Function for releasing TX or RX buffer.
 */
static __INLINE void spi_master_buffer_release(uint8_t * volatile * const pp_buf,
                                               volatile uint16_t * const  p_buf_len)
{
    APP_ERROR_CHECK_BOOL(pp_buf != NULL);
    APP_ERROR_CHECK_BOOL(p_buf_len != NULL);

    *pp_buf    = NULL;
    *p_buf_len = 0;
}

/**
 * @brief Function for sending events by callback.
 */
static __INLINE void spi_master_signal_evt(volatile spi_master_instance_t * const p_spi_instance,
                                           spi_master_evt_type_t                  event_type,
                                           const uint16_t                         data_count)
{
    APP_ERROR_CHECK_BOOL(p_spi_instance != NULL);

    if (p_spi_instance->callback_event_handler != NULL)
    {
        spi_master_evt_t event = {SPI_MASTER_EVT_TYPE_MAX, 0};
        event.evt_type   = event_type;
        event.data_count = data_count;
        p_spi_instance->callback_event_handler(event);
    }
}

/**
 * @brief Function insert to a TX buffer another byte or two bytes (depends on flag @ref DOUBLE_BUFFERED).
 */
static __INLINE void spi_master_send_initial_bytes(
    volatile spi_master_instance_t * const p_spi_instance)
{
    APP_ERROR_CHECK_BOOL(p_spi_instance != NULL);
    
    uint8_t * p_tx_buffer = p_spi_instance->p_tx_buffer;
    uint16_t tx_length = p_spi_instance->tx_length;
    uint16_t tx_index = p_spi_instance->tx_index;

    p_spi_instance->p_nrf_spi->TXD = ((p_tx_buffer != NULL) && (tx_index  < tx_length)) ?
                                     p_tx_buffer[tx_index] : SPI_DEFAULT_TX_BYTE;
    tx_index = ++(p_spi_instance->tx_index);
     
    if (tx_index < p_spi_instance->max_length)
    {
        p_spi_instance->p_nrf_spi->TXD = ((p_tx_buffer != NULL) && (tx_index < tx_length)) ?
                                         p_tx_buffer[tx_index] : SPI_DEFAULT_TX_BYTE;
        (p_spi_instance->tx_index)++;
    }
}

/**
 * @brief Function for receiving and sending data from IRQ. (The same for both IRQs).
 */
static __INLINE void spi_master_send_recv_irq(volatile spi_master_instance_t * const p_spi_instance)
{
    APP_ERROR_CHECK_BOOL(p_spi_instance != NULL);
    
    p_spi_instance->bytes_count++;

    if (!p_spi_instance->started_flag)
    {
        p_spi_instance->started_flag = true;
        spi_master_signal_evt(p_spi_instance,
                              SPI_MASTER_EVT_TRANSFER_STARTED,
                              p_spi_instance->bytes_count);
    }
    
    uint8_t rx_byte = p_spi_instance->p_nrf_spi->RXD;
    
    uint8_t * p_rx_buffer = p_spi_instance->p_rx_buffer;
    uint16_t rx_length    = p_spi_instance->rx_length;
    uint16_t rx_index     = p_spi_instance->rx_index;
    
    if ((p_rx_buffer != NULL) && (rx_index < rx_length))
    {
        p_rx_buffer[p_spi_instance->rx_index++] = rx_byte;
    }
    
    uint8_t * p_tx_buffer = p_spi_instance->p_tx_buffer;
    uint16_t tx_length    = p_spi_instance->tx_length;
    uint16_t tx_index     = p_spi_instance->tx_index;
    uint16_t max_length   = p_spi_instance->max_length;
    
    if (tx_index < max_length)
    {
        p_spi_instance->p_nrf_spi->TXD = ((p_tx_buffer != NULL) && (tx_index < tx_length)) ?
                                         p_tx_buffer[tx_index] : SPI_DEFAULT_TX_BYTE;
        (p_spi_instance->tx_index)++;
    }
    
    if (p_spi_instance->bytes_count >= max_length)
    {
        nrf_gpio_pin_set(p_spi_instance->pin_slave_select);

        uint16_t transmited_bytes = p_spi_instance->tx_index;

        spi_master_buffer_release(&(p_spi_instance->p_tx_buffer), &(p_spi_instance->tx_length));
        spi_master_buffer_release(&(p_spi_instance->p_rx_buffer), &(p_spi_instance->rx_length));

        p_spi_instance->state = SPI_MASTER_STATE_IDLE;

        spi_master_signal_evt(p_spi_instance, SPI_MASTER_EVT_TRANSFER_COMPLETED, transmited_bytes);
    }
}
#endif //defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE)
uint32_t spi_master_open(const spi_master_hw_instance_t    spi_master_hw_instance,
                         spi_master_config_t const * const p_spi_master_config)
{
    #if defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE)

    /* Check against null */
    if (p_spi_master_config == NULL)
    {
        return NRF_ERROR_NULL;
    }

    volatile spi_master_instance_t * p_spi_instance = spi_master_get_instance(
        spi_master_hw_instance);
    APP_ERROR_CHECK_BOOL(p_spi_instance != NULL);

    switch (spi_master_hw_instance)
    {
    #ifdef SPI_MASTER_0_ENABLE
        case SPI_MASTER_0:
            spi_master_init_hw_instance(NRF_SPI0, SPI0_TWI0_IRQn, p_spi_instance, p_spi_master_config->SPI_DisableAllIRQ);
            break;
    #endif /* SPI_MASTER_0_ENABLE */

    #ifdef SPI_MASTER_1_ENABLE
        case SPI_MASTER_1:
            spi_master_init_hw_instance(NRF_SPI1, SPI1_TWI1_IRQn, p_spi_instance, p_spi_master_config->SPI_DisableAllIRQ);
            break;
    #endif /* SPI_MASTER_1_ENABLE */

        default:
            break;
    }

    //A Slave select must be set as high before setting it as output,
    //because during connect it to the pin it causes glitches.
    nrf_gpio_pin_set(p_spi_master_config->SPI_Pin_SS);
    nrf_gpio_cfg_output(p_spi_master_config->SPI_Pin_SS);
    nrf_gpio_pin_set(p_spi_master_config->SPI_Pin_SS);

    //Configure GPIO
    nrf_gpio_cfg_output(p_spi_master_config->SPI_Pin_SCK);
    nrf_gpio_cfg_output(p_spi_master_config->SPI_Pin_MOSI);
    nrf_gpio_cfg_input(p_spi_master_config->SPI_Pin_MISO, NRF_GPIO_PIN_NOPULL);
    p_spi_instance->pin_slave_select = p_spi_master_config->SPI_Pin_SS;

    /* Configure SPI hardware */
    p_spi_instance->p_nrf_spi->PSELSCK  = p_spi_master_config->SPI_Pin_SCK;
    p_spi_instance->p_nrf_spi->PSELMOSI = p_spi_master_config->SPI_Pin_MOSI;
    p_spi_instance->p_nrf_spi->PSELMISO = p_spi_master_config->SPI_Pin_MISO;

    p_spi_instance->p_nrf_spi->FREQUENCY = p_spi_master_config->SPI_Freq;

    p_spi_instance->p_nrf_spi->CONFIG =
        (uint32_t)(p_spi_master_config->SPI_CONFIG_CPHA << SPI_CONFIG_CPHA_Pos) |
        (p_spi_master_config->SPI_CONFIG_CPOL << SPI_CONFIG_CPOL_Pos) |
        (p_spi_master_config->SPI_CONFIG_ORDER << SPI_CONFIG_ORDER_Pos);

    /* Clear waiting interrupts and events */
    p_spi_instance->p_nrf_spi->EVENTS_READY = 0;

    APP_ERROR_CHECK(sd_nvic_ClearPendingIRQ(p_spi_instance->irq_type));
    APP_ERROR_CHECK(sd_nvic_SetPriority(p_spi_instance->irq_type, p_spi_master_config->SPI_PriorityIRQ));

    /* Clear event handler */
    p_spi_instance->callback_event_handler = NULL;

    /* Enable interrupt */
    p_spi_instance->p_nrf_spi->INTENSET = (SPI_INTENSET_READY_Set << SPI_INTENCLR_READY_Pos);
    APP_ERROR_CHECK(sd_nvic_EnableIRQ(p_spi_instance->irq_type));

    /* Enable SPI hardware */
    p_spi_instance->p_nrf_spi->ENABLE = (SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);

    /* Change state to IDLE */
    p_spi_instance->state = SPI_MASTER_STATE_IDLE;

    return NRF_SUCCESS;
    #else
    return NRF_ERROR_NOT_SUPPORTED;
    #endif
}

void spi_master_close(const spi_master_hw_instance_t spi_master_hw_instance)
{
    #if defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE)

    volatile spi_master_instance_t * p_spi_instance = spi_master_get_instance(
        spi_master_hw_instance);
    APP_ERROR_CHECK_BOOL(p_spi_instance != NULL);

    /* Disable interrupt */
    APP_ERROR_CHECK(sd_nvic_ClearPendingIRQ(p_spi_instance->irq_type));
    APP_ERROR_CHECK(sd_nvic_DisableIRQ(p_spi_instance->irq_type));

    p_spi_instance->p_nrf_spi->ENABLE = (SPI_ENABLE_ENABLE_Disabled << SPI_ENABLE_ENABLE_Pos);

    /* Set Slave Select pin as input with pull-up. */
    nrf_gpio_pin_set(p_spi_instance->pin_slave_select);
    nrf_gpio_cfg_input(p_spi_instance->pin_slave_select, NRF_GPIO_PIN_PULLUP);
    p_spi_instance->pin_slave_select = (uint8_t)0xFF;

    /* Disconnect pins from SPI hardware */
    p_spi_instance->p_nrf_spi->PSELSCK  = (uint32_t)SPI_PIN_DISCONNECTED;
    p_spi_instance->p_nrf_spi->PSELMOSI = (uint32_t)SPI_PIN_DISCONNECTED;
    p_spi_instance->p_nrf_spi->PSELMISO = (uint32_t)SPI_PIN_DISCONNECTED;

    /* Reset to default values */
    spi_master_init_hw_instance(NULL, (IRQn_Type)0, p_spi_instance, false);
    #else
    return;
    #endif
}

__INLINE spi_master_state_t spi_master_get_state(
    const spi_master_hw_instance_t spi_master_hw_instance)
{
    #if defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE)
    volatile spi_master_instance_t * p_spi_instance = spi_master_get_instance(
        spi_master_hw_instance);
    APP_ERROR_CHECK_BOOL(p_spi_instance != NULL);

    return p_spi_instance->state;
    #else
    return SPI_MASTER_STATE_DISABLED;
    #endif
}

__INLINE void spi_master_evt_handler_reg(const spi_master_hw_instance_t spi_master_hw_instance,
                                         spi_master_event_handler_t     event_handler)
{
    #if defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE)
    volatile spi_master_instance_t * p_spi_instance = spi_master_get_instance(
        spi_master_hw_instance);
    APP_ERROR_CHECK_BOOL(p_spi_instance != NULL);

    p_spi_instance->callback_event_handler = event_handler;
    #else
    return;
    #endif
}

uint32_t spi_master_send_recv(const spi_master_hw_instance_t spi_master_hw_instance,
                              uint8_t * const p_tx_buf, const uint16_t tx_buf_len,
                              uint8_t * const p_rx_buf, const uint16_t rx_buf_len)
{
    #if defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE)

    volatile spi_master_instance_t * p_spi_instance = spi_master_get_instance(
        spi_master_hw_instance);
    APP_ERROR_CHECK_BOOL(p_spi_instance != NULL);

    uint32_t err_code   = NRF_SUCCESS;
    uint16_t max_length = 0;
    
    uint8_t nested_critical_region = 0;
    
    //Check if disable all IRQs flag is set
    if (p_spi_instance->disable_all_irq)
    {
        //Disable interrupts.
        APP_ERROR_CHECK(sd_nvic_critical_region_enter(&nested_critical_region));
    }
    else
    {
        //Disable interrupt SPI.
        APP_ERROR_CHECK(sd_nvic_DisableIRQ(p_spi_instance->irq_type));
    }

    //Initialize and perform data transfer
    if (p_spi_instance->state == SPI_MASTER_STATE_IDLE)
    {
        max_length = (rx_buf_len > tx_buf_len) ? rx_buf_len : tx_buf_len;

        if (max_length > 0)
        {
            p_spi_instance->state        = SPI_MASTER_STATE_BUSY;
            p_spi_instance->bytes_count  = 0;
            p_spi_instance->started_flag = false;
            p_spi_instance->max_length   = max_length;

            /* Initialize buffers */
            spi_master_buffer_init(p_tx_buf,
                                   tx_buf_len,
                                   &(p_spi_instance->p_tx_buffer),
                                   &(p_spi_instance->tx_length),
                                   &(p_spi_instance->tx_index));
            spi_master_buffer_init(p_rx_buf,
                                   rx_buf_len,
                                   &(p_spi_instance->p_rx_buffer),
                                   &(p_spi_instance->rx_length),
                                   &(p_spi_instance->rx_index));

            nrf_gpio_pin_clear(p_spi_instance->pin_slave_select);
            spi_master_send_initial_bytes(p_spi_instance);
        }
        else
        {
            err_code = NRF_ERROR_INVALID_PARAM;
        }
    }
    else
    {
        err_code = NRF_ERROR_BUSY;
    }
    
    //Check if disable all IRQs flag is set.
    if (p_spi_instance->disable_all_irq)
    {   
        //Enable interrupts.
        APP_ERROR_CHECK(sd_nvic_critical_region_exit(nested_critical_region));
    }
    else
    {
        //Enable SPI interrupt.
        APP_ERROR_CHECK(sd_nvic_EnableIRQ(p_spi_instance->irq_type));
    }

    return err_code;
    #else
    return NRF_ERROR_NOT_SUPPORTED;
    #endif
}
