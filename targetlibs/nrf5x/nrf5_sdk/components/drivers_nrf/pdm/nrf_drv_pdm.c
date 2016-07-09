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

#include "nrf_drv_pdm.h"
#include "nrf_assert.h"
#include "nrf_drv_common.h"
#include "nrf_gpio.h"


/** @brief PDM interface status. */
typedef enum
{
    NRF_PDM_STATE_IDLE,
    NRF_PDM_STATE_RUNNING,
    NRF_PDM_STATE_TRANSITION
} nrf_drv_pdm_state_t;


/** @brief PDM interface control block.*/
typedef struct
{
    nrf_drv_state_t             drv_state;        ///< Driver state.
    nrf_drv_pdm_state_t         status;           ///< Sampling state.
    nrf_drv_pdm_event_handler_t event_handler;    ///< Event handler function pointer.
    uint16_t                    buffer_length;    ///< Length of a single buffer in 16-bit words.
    uint32_t *                  buffers[2];       ///< Sample buffers.
} nrf_drv_pdm_cb_t;

static nrf_drv_pdm_cb_t m_cb;


void PDM_IRQHandler(void)
{
    if (nrf_pdm_event_check(NRF_PDM_EVENT_END))
    {
        nrf_pdm_event_clear(NRF_PDM_EVENT_END);
        
        //Buffer is ready to process.
        if (nrf_pdm_buffer_get() == m_cb.buffers[0])
        {
            m_cb.event_handler(m_cb.buffers[1], m_cb.buffer_length);
        }
        else
        {
            m_cb.event_handler(m_cb.buffers[0], m_cb.buffer_length);
        }
    }
    else if (nrf_pdm_event_check(NRF_PDM_EVENT_STARTED))
    {
        nrf_pdm_event_clear(NRF_PDM_EVENT_STARTED);
        m_cb.status = NRF_PDM_STATE_RUNNING;
        
        //Swap buffer.
        if (nrf_pdm_buffer_get() == m_cb.buffers[0])
        {
            nrf_pdm_buffer_set(m_cb.buffers[1],m_cb.buffer_length);
        }
        else
        {
            nrf_pdm_buffer_set(m_cb.buffers[0],m_cb.buffer_length);
        }
    }
    else if (nrf_pdm_event_check(NRF_PDM_EVENT_STOPPED))
    {
        nrf_pdm_event_clear(NRF_PDM_EVENT_STOPPED);
        nrf_pdm_disable();
        m_cb.status = NRF_PDM_STATE_IDLE;
    }
}


ret_code_t nrf_drv_pdm_init(nrf_drv_pdm_config_t const * p_config,
                              nrf_drv_pdm_event_handler_t event_handler)
{
    if (m_cb.drv_state != NRF_DRV_STATE_UNINITIALIZED)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    if ((p_config == NULL)
        || (event_handler == NULL))
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    if (p_config->gain_l > NRF_PDM_GAIN_MAXIMUM
        || p_config->gain_r > NRF_PDM_GAIN_MAXIMUM
        || p_config->buffer_length > NRF_PDM_MAX_BUFFER_SIZE)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    
    m_cb.buffers[0] = (uint32_t*)p_config->buffer_a;
    m_cb.buffers[1] = (uint32_t*)p_config->buffer_b;
    m_cb.buffer_length = p_config->buffer_length;
    m_cb.event_handler = event_handler;
    m_cb.status = NRF_PDM_STATE_IDLE;
    
    nrf_pdm_buffer_set(m_cb.buffers[0],m_cb.buffer_length);
    nrf_pdm_clock_set(p_config->clock_freq);
    nrf_pdm_mode_set(p_config->mode, p_config->edge);
    nrf_pdm_gain_set(p_config->gain_l, p_config->gain_r);

    nrf_gpio_cfg_output(p_config->pin_clk);
    nrf_gpio_pin_clear(p_config->pin_clk);
    nrf_gpio_cfg_input(p_config->pin_din, NRF_GPIO_PIN_NOPULL);
    nrf_pdm_psel_connect(p_config->pin_clk, p_config->pin_din);
    
    m_cb.drv_state = NRF_DRV_STATE_INITIALIZED;
    nrf_pdm_int_enable(NRF_PDM_INT_STARTED | NRF_PDM_INT_END | NRF_PDM_INT_STOPPED);
    nrf_drv_common_irq_enable(PDM_IRQn, p_config->interrupt_priority);
    
    return NRF_SUCCESS;
}


void nrf_drv_pdm_uninit(void)
{
    nrf_pdm_disable();
    nrf_pdm_psel_disconnect();
    m_cb.drv_state = NRF_DRV_STATE_UNINITIALIZED;
}




ret_code_t nrf_drv_pdm_start(void)
{
    ASSERT(m_cb.drv_state != NRF_DRV_STATE_UNINITIALIZED);
    if (m_cb.status != NRF_PDM_STATE_IDLE)
    {
        if (m_cb.status == NRF_PDM_STATE_RUNNING)
        {
            return NRF_SUCCESS;
        }
        return NRF_ERROR_BUSY;
    }
    m_cb.status = NRF_PDM_STATE_TRANSITION;
    m_cb.drv_state = NRF_DRV_STATE_POWERED_ON;
    nrf_pdm_enable();
    nrf_pdm_event_clear(NRF_PDM_EVENT_STARTED);
    nrf_pdm_task_trigger(NRF_PDM_TASK_START);
    return NRF_SUCCESS;
}


ret_code_t nrf_drv_pdm_stop(void)
{
    ASSERT(m_cb.drv_state != NRF_DRV_STATE_UNINITIALIZED);
    if (m_cb.status != NRF_PDM_STATE_RUNNING)
    {
        if (m_cb.status == NRF_PDM_STATE_IDLE)
        {
            nrf_pdm_disable();
            return NRF_SUCCESS;
        }
        return NRF_ERROR_BUSY;
    }
    m_cb.status = NRF_PDM_STATE_TRANSITION;
    m_cb.drv_state = NRF_DRV_STATE_INITIALIZED;
    nrf_pdm_task_trigger(NRF_PDM_TASK_STOP);
    return NRF_SUCCESS;
}
