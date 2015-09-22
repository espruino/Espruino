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

#include "nrf_drv_timer.h"
#include "nrf_assert.h"
#include "nrf_drv_common.h"
#include "app_util_platform.h"

/**@brief Timer control block. */
typedef struct
{
    nrf_drv_state_t       state;
} nrf_drv_timer_cb_t;

/**@brief Array for storing timers event handlers. */
static nrf_timer_event_handler_t m_timer_event_handlers[TIMER_COUNT];
static nrf_drv_timer_cb_t        m_cb[TIMER_COUNT];
static void*                     mp_contexts[TIMER_COUNT];

static const nrf_drv_timer_config_t m_default_config[] = {
#if (TIMER0_ENABLED == 1)
    NRF_DRV_TIMER_DEFAULT_CONFIG(0),
#endif
#if (TIMER1_ENABLED == 1)
    NRF_DRV_TIMER_DEFAULT_CONFIG(1),
#endif
#if (TIMER2_ENABLED == 1)
    NRF_DRV_TIMER_DEFAULT_CONFIG(2)
#endif
};

ret_code_t nrf_drv_timer_init(nrf_drv_timer_t const * const p_instance,
                              nrf_drv_timer_config_t const * p_config,
                              nrf_timer_event_handler_t timer_event_handler)
{
    ASSERT((p_instance->instance_id) < TIMER_INSTANCE_NUMBER);
    ASSERT(TIMER_IS_BIT_WIDTH_VALID(p_instance->instance_id, p_config->bit_width));

    if (m_cb[p_instance->instance_id].state != NRF_DRV_STATE_UNINITIALIZED)
    {
        return NRF_ERROR_INVALID_STATE; // timer already initialized
    } 

    if (p_config == NULL)
    {
        p_config = &m_default_config[p_instance->instance_id];
    }

#ifdef SOFTDEVICE_PRESENT
    if (p_instance->p_reg == NRF_TIMER0)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
#endif    
    
    nrf_drv_common_irq_enable(p_instance->irq, p_config->interrupt_priority);
    
    mp_contexts[p_instance->instance_id] = p_config->p_context;
    
    if (timer_event_handler != NULL)
    {
        m_timer_event_handlers[p_instance->instance_id] = timer_event_handler;
    }
    else
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    nrf_timer_mode_set(p_instance->p_reg, p_config->mode);
    nrf_timer_bit_width_set(p_instance->p_reg, p_config->bit_width);
    nrf_timer_frequency_set(p_instance->p_reg, p_config->frequency);

    m_cb[p_instance->instance_id].state = NRF_DRV_STATE_INITIALIZED;

    return NRF_SUCCESS;
}

void nrf_drv_timer_uninit(nrf_drv_timer_t const * const p_instance)
{
    uint32_t i;

    nrf_drv_common_irq_disable(p_instance->irq);

    m_timer_event_handlers[p_instance->instance_id] = NULL;

    nrf_drv_timer_disable(p_instance);
    
    //lint -save -e655
    nrf_timer_shorts_disable(p_instance->p_reg, NRF_TIMER_SHORT_COMPARE0_STOP_MASK  |
                                           NRF_TIMER_SHORT_COMPARE1_STOP_MASK  |
                                           NRF_TIMER_SHORT_COMPARE2_STOP_MASK  |
                                           NRF_TIMER_SHORT_COMPARE3_STOP_MASK  |
                                           NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK |
                                           NRF_TIMER_SHORT_COMPARE1_CLEAR_MASK |
                                           NRF_TIMER_SHORT_COMPARE2_CLEAR_MASK |
                                           NRF_TIMER_SHORT_COMPARE3_CLEAR_MASK);
    //lint -restore
    
    for(i=0; i<TIMER_CHANNEL_NUMBER; i++)
    {   
        nrf_timer_int_disable(p_instance->p_reg, NRF_TIMER_INT_COMPARE0_MASK << i);
    }
    
    m_cb[p_instance->instance_id].state = NRF_DRV_STATE_UNINITIALIZED;
}

void nrf_drv_timer_enable(nrf_drv_timer_t const * const p_instance)
{
    ASSERT(m_cb[p_instance->instance_id].state == NRF_DRV_STATE_INITIALIZED);
    nrf_timer_task_trigger(p_instance->p_reg, NRF_TIMER_TASK_START);
    m_cb[p_instance->instance_id].state = NRF_DRV_STATE_POWERED_ON;
}

void nrf_drv_timer_disable(nrf_drv_timer_t const * const p_instance)
{
    ASSERT(m_cb[p_instance->instance_id].state == NRF_DRV_STATE_POWERED_ON);
    nrf_timer_task_trigger(p_instance->p_reg, NRF_TIMER_TASK_SHUTDOWN);
    m_cb[p_instance->instance_id].state = NRF_DRV_STATE_INITIALIZED;
}

void nrf_drv_timer_resume(nrf_drv_timer_t const * const p_instance)
{
    ASSERT(m_cb[p_instance->instance_id].state == NRF_DRV_STATE_POWERED_ON);
    nrf_timer_task_trigger(p_instance->p_reg, NRF_TIMER_TASK_START);
}

void nrf_drv_timer_pause(nrf_drv_timer_t const * const p_instance)
{
    ASSERT(m_cb[p_instance->instance_id].state == NRF_DRV_STATE_POWERED_ON);
    nrf_timer_task_trigger(p_instance->p_reg, NRF_TIMER_TASK_STOP);
}

void nrf_drv_timer_clear(nrf_drv_timer_t const * const p_instance)
{
    ASSERT(m_cb[p_instance->instance_id].state != NRF_DRV_STATE_UNINITIALIZED);
    nrf_timer_task_trigger(p_instance->p_reg, NRF_TIMER_TASK_CLEAR);
}

void nrf_drv_timer_increment(nrf_drv_timer_t const * const p_instance)
{
    ASSERT(m_cb[p_instance->instance_id].state == NRF_DRV_STATE_POWERED_ON);
    ASSERT(nrf_timer_mode_get(p_instance->p_reg) == NRF_TIMER_MODE_COUNTER);
    
    nrf_timer_task_trigger(p_instance->p_reg, NRF_TIMER_TASK_COUNT);
}

uint32_t nrf_drv_timer_task_address_get(nrf_drv_timer_t const * const p_instance,
                                        nrf_timer_task_t              timer_task)
{
    return (uint32_t)nrf_timer_task_address_get(p_instance->p_reg, timer_task);
}

uint32_t nrf_drv_timer_event_address_get(nrf_drv_timer_t const * const p_instance,
                                         nrf_timer_event_t             timer_event)
{
    return (uint32_t)nrf_timer_event_address_get(p_instance->p_reg, timer_event);
}

/**
 * @brief Function for getting the specific timer capture task.
 *
 * @param[in]  channel    Capture channel number.
 *
 * @retval     Capture task.
 */
__STATIC_INLINE nrf_timer_task_t nrf_drv_timer_capture_task_get(uint32_t channel)
{
    /* nrf_timer_tasks_t stores offset value and distance between two tasks equals to sizeof(uint32_t) = 4 */
    return (nrf_timer_task_t)((uint32_t) NRF_TIMER_TASK_CAPTURE0 + (channel * sizeof(uint32_t)));
}

/**
 * @brief Function for getting the specific timer compare event.
 *
 * @param[in]  channel    Compare channel number.
 *
 * @retval     Compare event.
 */
__STATIC_INLINE nrf_timer_event_t nrf_drv_timer_compare_event_get(uint32_t channel)
{
    return (nrf_timer_event_t)((uint32_t) NRF_TIMER_EVENT_COMPARE0 + (channel * sizeof(uint32_t)));
}

uint32_t nrf_drv_timer_capture_task_address_get(nrf_drv_timer_t const * const p_instance,
                                                uint32_t channel)
{
    ASSERT(channel < TIMER_CHANNEL_NUMBER);
    return (uint32_t)nrf_timer_task_address_get(p_instance->p_reg,
                                                nrf_drv_timer_capture_task_get(channel));
}

uint32_t nrf_drv_timer_compare_event_address_get(nrf_drv_timer_t const * const p_instance,
                                                 uint32_t channel)
{
    ASSERT(channel < TIMER_CHANNEL_NUMBER);
    return (uint32_t)nrf_timer_event_address_get(p_instance->p_reg,
                                                 nrf_drv_timer_compare_event_get(channel) );
}

uint32_t nrf_drv_timer_capture(nrf_drv_timer_t const * const p_instance,
                               nrf_timer_cc_channel_t        cc_channel)
{
    ASSERT(m_cb[p_instance->instance_id].state == NRF_DRV_STATE_POWERED_ON);
    ASSERT(cc_channel < TIMER_CHANNEL_NUMBER);
    
    /*lint -save -e644*/
    nrf_timer_task_trigger(p_instance->p_reg, nrf_drv_timer_capture_task_get(cc_channel));
    /*lint -restore*/
    return nrf_timer_cc_read(p_instance->p_reg, cc_channel);
}

uint32_t nrf_drv_timer_capture_get(nrf_drv_timer_t const * const p_instance,
                                   nrf_timer_cc_channel_t        cc_channel)
{
    return nrf_timer_cc_read(p_instance->p_reg, cc_channel);
}

void nrf_drv_timer_compare(nrf_drv_timer_t const * const p_instance,
                           nrf_timer_cc_channel_t        cc_channel,
                           uint32_t                      cc_value,
                           bool                          enable)
{
    nrf_timer_int_mask_t timer_int;

    timer_int = (nrf_timer_int_mask_t)((uint32_t)NRF_TIMER_INT_COMPARE0_MASK << cc_channel);

    if (enable == true)
    {
        /*lint -save -e644*/
        nrf_timer_int_enable(p_instance->p_reg, timer_int);
        /*lint -restore*/
    }
    else
    {
        nrf_timer_int_disable(p_instance->p_reg, timer_int);
    }

    nrf_timer_cc_write(p_instance->p_reg, cc_channel, cc_value);
}

void nrf_drv_timer_extended_compare(nrf_drv_timer_t const * const p_instance,
                                        nrf_timer_cc_channel_t        cc_channel,
                                        uint32_t                      cc_value,
                                        nrf_timer_short_mask_t        timer_short_mask,
                                        bool                          enable)
{
    nrf_timer_shorts_disable(p_instance->p_reg, TIMER_CC_SHORT(cc_channel));
    nrf_timer_shorts_enable(p_instance->p_reg, timer_short_mask);

    (void)nrf_drv_timer_compare(p_instance,
                                cc_channel,
                                cc_value,
                                enable);
}

uint32_t nrf_drv_timer_us_to_ticks(nrf_drv_timer_t const * const p_instance,
                                   uint32_t                      time_us)
{
    uint32_t multiplier = 512UL;                //must be divisible by 2^(PRESCALER_MAX)
    uint32_t prescaler_value = nrf_timer_frequency_get(p_instance->p_reg);
    ASSERT(prescaler_value <= 9);               //maximum prescaler value
    ASSERT((UINT32_MAX / (multiplier >> prescaler_value)) > time_us);
	  return (((multiplier >> prescaler_value) * time_us) / 32UL);    //32 -> multiplier divided by clock freq. in MHz
}


uint32_t nrf_drv_timer_ms_to_ticks(nrf_drv_timer_t const * const p_instance,
                                   uint32_t                      time_ms)
{
    uint32_t multiplier = 64000UL;              //must be divisible by 2^(PRESCALER_MAX)
    uint32_t prescaler_value = nrf_timer_frequency_get(p_instance->p_reg);
    ASSERT(prescaler_value <= 9);               //maximum prescaler value
    ASSERT((UINT32_MAX / (multiplier >> prescaler_value)) > time_ms);
    return (((multiplier >> prescaler_value) * time_ms) / 4UL);    //4 -> multiplier divided by clock freq. in kHz
}

void nrf_drv_timer_compare_int_enable(nrf_drv_timer_t const * const p_instance,
                                      uint32_t channel)
{
    ASSERT(m_cb[p_instance->instance_id].state != NRF_DRV_STATE_UNINITIALIZED);
    ASSERT(channel < TIMER_CHANNEL_NUMBER);
    nrf_timer_event_clear(p_instance->p_reg, nrf_drv_timer_compare_event_get(channel));
    nrf_timer_int_enable(p_instance->p_reg, (uint32_t) NRF_TIMER_INT_COMPARE0_MASK << channel);
}

void nrf_drv_timer_compare_int_disable(nrf_drv_timer_t const * const p_instance,
                                       uint32_t channel)
{
    ASSERT(m_cb[p_instance->instance_id].state != NRF_DRV_STATE_UNINITIALIZED);
    ASSERT(channel < TIMER_CHANNEL_NUMBER);
    nrf_timer_int_disable(p_instance->p_reg, (uint32_t) NRF_TIMER_INT_COMPARE0_MASK << channel);
}

/**
 * @brief This function is generic interrupt handler
 *
 * @param[in] p_reg    pointer to timer registers
 * @param[in] timer_id specifies the timer id
 *
 * @return    NRF_SUCCESS on success, otherwise an error code.
 */
static void nrf_drv_timer_interrupt_handle(NRF_TIMER_Type * p_reg, uint32_t timer_id)
{
    uint32_t i;

    for(i=0; i<TIMER_CHANNEL_NUMBER; i++)
    {
        nrf_timer_event_t event = nrf_drv_timer_compare_event_get(i);

        if (nrf_timer_event_check(p_reg, event)
            && nrf_timer_int_enable_check(p_reg, (nrf_timer_int_mask_t) ((uint32_t )NRF_TIMER_INT_COMPARE0_MASK << i)))
        {
            nrf_timer_event_clear(p_reg, event);
            (m_timer_event_handlers[timer_id])(event, mp_contexts[timer_id]);
        }
    }
}

#if TIMER0_ENABLED == 1
void TIMER0_IRQHandler(void)
{
    nrf_drv_timer_interrupt_handle(NRF_TIMER0, TIMER0_INSTANCE_INDEX);
}
#endif

#if TIMER1_ENABLED == 1
void TIMER1_IRQHandler(void)
{
    nrf_drv_timer_interrupt_handle(NRF_TIMER1, TIMER1_INSTANCE_INDEX);
}
#endif

#if TIMER2_ENABLED == 1
void TIMER2_IRQHandler(void)
{
    nrf_drv_timer_interrupt_handle(NRF_TIMER2, TIMER2_INSTANCE_INDEX);
}
#endif

#if TIMER3_ENABLED == 1
void TIMER3_IRQHandler(void)
{
    nrf_drv_timer_interrupt_handle(NRF_TIMER3, TIMER3_INSTANCE_INDEX);
}
#endif

#if TIMER4_ENABLED == 1
void TIMER4_IRQHandler(void)
{
    nrf_drv_timer_interrupt_handle(NRF_TIMER4, TIMER4_INSTANCE_INDEX);
}
#endif
