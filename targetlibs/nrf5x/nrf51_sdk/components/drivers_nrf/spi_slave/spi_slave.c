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
 
#include "spi_slave.h"
#include <stdbool.h>
#include <stdio.h>
#include "nrf51.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "app_util_platform.h"

#define SPI1_TWI1_IRQ_PRI           APP_IRQ_PRIORITY_LOW      /**< Priority of the SPI slave device interrupt. */
#define DEFAULT_CS_PULLUP_CONFIG    GPIO_PIN_CNF_PULL_Disabled /**< Default Pull configuration of SPI CS. */
#define DEFAULT_DRIVE_CONFIG        GPIO_PIN_CNF_DRIVE_S0S1    /**< Default drive configuration of SPI MISO. */

static uint32_t m_cs_pullup_config = DEFAULT_CS_PULLUP_CONFIG; /**< SPI CS pin Pull configuration. */
static uint32_t m_drive_config = GPIO_PIN_CNF_DRIVE_S0S1;      /**< SPI MISO pin output drive configuration. */

/**@brief States of the SPI transaction state machine. */
typedef enum
{
    SPI_STATE_INIT,                                 /**< Initialization state. In this state the module waits for a call to @ref spi_slave_buffers_set. */                                                                                             
    SPI_BUFFER_RESOURCE_REQUESTED,                  /**< State where the configuration of the memory buffers, which are to be used in SPI transaction, has started. */
    SPI_BUFFER_RESOURCE_CONFIGURED,                 /**< State where the configuration of the memory buffers, which are to be used in SPI transaction, has completed. */
    SPI_XFER_COMPLETED                              /**< State where SPI transaction has been completed. */
} spi_state_t;

static volatile uint8_t *   mp_spi_tx_buf;          /**< SPI slave TX buffer. */
static volatile uint8_t *   mp_spi_rx_buf;          /**< SPI slave RX buffer. */
static volatile uint32_t    m_spi_tx_buf_size;      /**< SPI slave TX buffer size in bytes. */
static volatile uint32_t    m_spi_rx_buf_size;      /**< SPI slave RX buffer size in bytes. */
static volatile spi_state_t m_spi_state;            /**< SPI slave state. */

static spi_slave_event_handler_t m_event_callback;  /**< SPI slave event callback function. */

uint32_t spi_slave_evt_handler_register(spi_slave_event_handler_t event_handler)
{
    m_event_callback = event_handler;
    
    return (m_event_callback != NULL) ? NRF_SUCCESS : NRF_ERROR_NULL;
}


uint32_t spi_slave_set_cs_pull_up_config(uint32_t alternate_config)
{
    m_cs_pullup_config = alternate_config;
    return NRF_SUCCESS;
}

uint32_t spi_slave_set_drive_config(uint32_t alternate_config)
{
    m_drive_config = alternate_config;
    return NRF_SUCCESS;
}

uint32_t spi_slave_init(const spi_slave_config_t * p_spi_slave_config)
{    
    uint32_t err_code;
    uint32_t spi_mode_mask;
    
    if (p_spi_slave_config == NULL)
    {
        return NRF_ERROR_NULL;
    }

    // Configure the SPI pins for input.
    NRF_GPIO->PIN_CNF[p_spi_slave_config->pin_miso] = 
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
        (m_drive_config << GPIO_PIN_CNF_DRIVE_Pos)              |
        (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)   |
        (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)  |
        (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);

    NRF_GPIO->PIN_CNF[p_spi_slave_config->pin_csn] = 
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
        (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)     |
        (m_cs_pullup_config << GPIO_PIN_CNF_PULL_Pos)           |
        (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)  |
        (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);

    NRF_GPIO->PIN_CNF[p_spi_slave_config->pin_mosi] = 
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
        (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)     |
        (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)   |
        (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)  |
        (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);

    NRF_GPIO->PIN_CNF[p_spi_slave_config->pin_sck] = 
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
        (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)     |
        (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)   |
        (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)  |
        (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);

    NRF_SPIS1->PSELCSN  = p_spi_slave_config->pin_csn;
    NRF_SPIS1->PSELSCK  = p_spi_slave_config->pin_sck;
    NRF_SPIS1->PSELMOSI = p_spi_slave_config->pin_mosi;
    NRF_SPIS1->PSELMISO = p_spi_slave_config->pin_miso;
    NRF_SPIS1->MAXRX    = 0;
    NRF_SPIS1->MAXTX    = 0;
    
    // Configure SPI mode.
    spi_mode_mask = 0;
    err_code      = NRF_SUCCESS;
    switch (p_spi_slave_config->mode)
    {
        case SPI_MODE_0:
            spi_mode_mask = ((SPIS_CONFIG_CPOL_ActiveHigh << SPIS_CONFIG_CPOL_Pos) |
                             (SPIS_CONFIG_CPHA_Leading << SPIS_CONFIG_CPHA_Pos));
            break;

        case SPI_MODE_1:
            spi_mode_mask = ((SPIS_CONFIG_CPOL_ActiveHigh << SPIS_CONFIG_CPOL_Pos) |
                             (SPIS_CONFIG_CPHA_Trailing << SPIS_CONFIG_CPHA_Pos));
            break;

        case SPI_MODE_2:
            spi_mode_mask = ((SPIS_CONFIG_CPOL_ActiveLow << SPIS_CONFIG_CPOL_Pos)  |
                             (SPIS_CONFIG_CPHA_Leading << SPIS_CONFIG_CPHA_Pos));
            break;

        case SPI_MODE_3:
            spi_mode_mask = ((SPIS_CONFIG_CPOL_ActiveLow << SPIS_CONFIG_CPOL_Pos)  |
                             (SPIS_CONFIG_CPHA_Trailing << SPIS_CONFIG_CPHA_Pos));
            break;

        default:
            err_code = NRF_ERROR_INVALID_PARAM;
            break;
    }

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    // Configure bit order.    
    if (p_spi_slave_config->bit_order == SPIM_LSB_FIRST)
    {
        NRF_SPIS1->CONFIG = (spi_mode_mask | (SPIS_CONFIG_ORDER_LsbFirst << SPIS_CONFIG_ORDER_Pos));
        NRF_SPIS1->DEF    = p_spi_slave_config->def_tx_character;
        NRF_SPIS1->ORC    = p_spi_slave_config->orc_tx_character;
    }
    else
    {
        NRF_SPIS1->CONFIG = (spi_mode_mask | (SPIS_CONFIG_ORDER_MsbFirst << SPIS_CONFIG_ORDER_Pos));
        NRF_SPIS1->DEF    = p_spi_slave_config->def_tx_character;
        NRF_SPIS1->ORC    = p_spi_slave_config->orc_tx_character;
    }

    // Clear possible pending events.
    NRF_SPIS1->EVENTS_END      = 0;
    NRF_SPIS1->EVENTS_ACQUIRED = 0;
    
    // Enable END_ACQUIRE shortcut.        
    NRF_SPIS1->SHORTS = (SPIS_SHORTS_END_ACQUIRE_Enabled << SPIS_SHORTS_END_ACQUIRE_Pos);
    
    m_spi_state = SPI_STATE_INIT; 

    // Set correct IRQ priority and clear any possible pending interrupt.
    NVIC_SetPriority(SPI1_TWI1_IRQn, SPI1_TWI1_IRQ_PRI);    
    NVIC_ClearPendingIRQ(SPI1_TWI1_IRQn);
    
    // Enable IRQ.    
    NRF_SPIS1->INTENSET = (SPIS_INTENSET_ACQUIRED_Enabled << SPIS_INTENSET_ACQUIRED_Pos) |
                          (SPIS_INTENSET_END_Enabled << SPIS_INTENSET_END_Pos);
    NVIC_EnableIRQ(SPI1_TWI1_IRQn);
    
    // Enable SPI slave device.        
    NRF_SPIS1->ENABLE = (SPIS_ENABLE_ENABLE_Enabled << SPIS_ENABLE_ENABLE_Pos);        
    
    return err_code;
}


/**@brief Function for executing the state entry action.
 */
static __INLINE void state_entry_action_execute(void)
{
    spi_slave_evt_t event;
    
    switch (m_spi_state)
    {                             
        case SPI_BUFFER_RESOURCE_REQUESTED:
            NRF_SPIS1->TASKS_ACQUIRE = 1u;                                  
            break;            
     
        case SPI_BUFFER_RESOURCE_CONFIGURED:
            event.evt_type  = SPI_SLAVE_BUFFERS_SET_DONE;
            event.rx_amount = 0;
            event.tx_amount = 0;     
            
            APP_ERROR_CHECK_BOOL(m_event_callback != NULL);
            m_event_callback(event);         
            break;
            
        case SPI_XFER_COMPLETED:        
            event.evt_type  = SPI_SLAVE_XFER_DONE;
            event.rx_amount = NRF_SPIS1->AMOUNTRX;
            event.tx_amount = NRF_SPIS1->AMOUNTTX;
            
            APP_ERROR_CHECK_BOOL(m_event_callback != NULL);
            m_event_callback(event);
            break;
            
        default:
            // No implementation required.            
            break;
    }
}


/**@brief Function for changing the state of the SPI state machine.
 *
 * @param[in] new_state State where the state machine transits to.
 */
static void sm_state_change(spi_state_t new_state)
{
    m_spi_state = new_state;
    state_entry_action_execute();
}


uint32_t spi_slave_buffers_set(uint8_t * p_tx_buf, 
                               uint8_t * p_rx_buf, 
                               uint8_t   tx_buf_length, 
                               uint8_t   rx_buf_length)
{
    uint32_t err_code;

    if ((p_tx_buf == NULL) || (p_rx_buf == NULL))
    {
        return NRF_ERROR_NULL;
    }
    
    switch (m_spi_state)
    {
        case SPI_STATE_INIT:
        case SPI_XFER_COMPLETED:
        case SPI_BUFFER_RESOURCE_CONFIGURED:        
            mp_spi_tx_buf     = p_tx_buf;
            mp_spi_rx_buf     = p_rx_buf;
            m_spi_tx_buf_size = tx_buf_length;
            m_spi_rx_buf_size = rx_buf_length;        
            err_code          = NRF_SUCCESS;            
                        
            sm_state_change(SPI_BUFFER_RESOURCE_REQUESTED);             
            break;

        case SPI_BUFFER_RESOURCE_REQUESTED:
            err_code = NRF_ERROR_INVALID_STATE; 
            break;
                        
        default:
            // @note: execution of this code path would imply internal error in the design.
            err_code = NRF_ERROR_INTERNAL;             
            break;
    }
    
    return err_code;
}


/**@brief SPI slave interrupt handler.
 *
 * SPI slave interrupt handler, which processes events generated by the SPI device.
 */
void SPI1_TWI1_IRQHandler(void)
{        
    // @note: as multiple events can be pending for processing, the correct event processing order 
    // is as follows:
    // - SPI semaphore acquired event.
    // - SPI transaction complete event.
    
    // Check for SPI semaphore acquired event.
    if (NRF_SPIS1->EVENTS_ACQUIRED != 0)
    {            
        NRF_SPIS1->EVENTS_ACQUIRED = 0;                     
        
        switch (m_spi_state)
        {                
            case SPI_BUFFER_RESOURCE_REQUESTED:                
                NRF_SPIS1->TXDPTR = (uint32_t)mp_spi_tx_buf;
                NRF_SPIS1->RXDPTR = (uint32_t)mp_spi_rx_buf;
                NRF_SPIS1->MAXRX  = m_spi_rx_buf_size;
                NRF_SPIS1->MAXTX  = m_spi_tx_buf_size;
                
                NRF_SPIS1->TASKS_RELEASE = 1u;
                
                sm_state_change(SPI_BUFFER_RESOURCE_CONFIGURED);                                                                         
                break;
                
            default:
                // No implementation required.                    
                break;
        }
    }

    // Check for SPI transaction complete event.
    if (NRF_SPIS1->EVENTS_END != 0)
    {
        NRF_SPIS1->EVENTS_END = 0;            
        
        switch (m_spi_state)
        {
            case SPI_BUFFER_RESOURCE_CONFIGURED:                                  
                sm_state_change(SPI_XFER_COMPLETED);                                                             
                break;

            default:
                // No implementation required.                    
                break;                
        }    
    }    
}
