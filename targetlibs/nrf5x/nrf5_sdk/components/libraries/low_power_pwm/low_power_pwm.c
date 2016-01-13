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
#include <string.h>
#include "low_power_pwm.h"
#include "nrf_gpio.h"
#include "app_timer.h"
#include "nrf_assert.h"

/**
 * @brief Function for turning on LEDs.
 *
 * Sets the pin high state according to active_high parameter.
 * 
 * @param[in] p_pwm_instance        Pointer to instance of low-power PWM.
 */
__STATIC_INLINE void led_on(low_power_pwm_t * p_pwm_instance)
{
    if (p_pwm_instance->active_high)
    {
        nrf_gpio_pins_set(p_pwm_instance->bit_mask);
    }
    else
    {
        nrf_gpio_pins_clear(p_pwm_instance->bit_mask);
    }
    p_pwm_instance->led_is_on = true;
}


/**
 * @brief Function for turning off LEDs.
 *
 * Sets the pin low state according to active_high parameter.
 *
 * @param[in] p_pwm_instance        Pointer to instance of low-power PWM.
 */
__STATIC_INLINE void led_off(low_power_pwm_t * p_pwm_instance)
{
    if (p_pwm_instance->active_high)
    {
        nrf_gpio_pins_clear(p_pwm_instance->bit_mask);
    }
    else
    {
        nrf_gpio_pins_set(p_pwm_instance->bit_mask);
    }
    p_pwm_instance->led_is_on = false;
}


/**
 * @brief Timer event handler for PWM.
 *
 * @param[in] p_context             General purpose pointer. Will be passed to the time-out handler
 *                                  when the timer expires.
 *
 */
static void pwm_timeout_handler(void * p_context)
{
    ret_code_t err_code;
    uint8_t duty_cycle;
    
    low_power_pwm_t * p_pwm_instance = (low_power_pwm_t *)p_context;
    
    p_pwm_instance->pwm_state = NRF_DRV_STATE_INITIALIZED;
    
    if(p_pwm_instance->evt_type == LOW_POWER_PWM_EVENT_PERIOD)
    {        
        if (p_pwm_instance->handler)
        {
            p_pwm_instance->handler(p_pwm_instance);
        }
        
        duty_cycle = p_pwm_instance->duty_cycle;
        
        if(duty_cycle == p_pwm_instance->period)    // Process duty cycle 100%
        {
            led_on(p_pwm_instance);
            p_pwm_instance->timeout_ticks = p_pwm_instance->period + APP_TIMER_MIN_TIMEOUT_TICKS;
        }
        else if (duty_cycle == 0)   // Process duty cycle 0%
        {
            led_off(p_pwm_instance);
            p_pwm_instance->timeout_ticks = p_pwm_instance->period + APP_TIMER_MIN_TIMEOUT_TICKS;
        }
        else // Process any other duty cycle than 0 or 100%
        {
            led_on(p_pwm_instance);
            p_pwm_instance->timeout_ticks = ((duty_cycle*p_pwm_instance->period)>>8) +
                                APP_TIMER_MIN_TIMEOUT_TICKS;
            // setting next state
            p_pwm_instance->evt_type = LOW_POWER_PWM_EVENT_DUTY_CYCLE;
        }
    }
    else
    {
        led_off(p_pwm_instance);
        p_pwm_instance->evt_type = LOW_POWER_PWM_EVENT_PERIOD;
        p_pwm_instance->timeout_ticks = (((p_pwm_instance->period - p_pwm_instance->duty_cycle)*p_pwm_instance->period)>>8) +
                                  APP_TIMER_MIN_TIMEOUT_TICKS;
    }
    
    if (p_pwm_instance->pwm_state == NRF_DRV_STATE_INITIALIZED)
    {
        err_code = app_timer_start(*p_pwm_instance->p_timer_id, p_pwm_instance->timeout_ticks, p_pwm_instance);
        APP_ERROR_CHECK(err_code);
    }
}


ret_code_t low_power_pwm_init(low_power_pwm_t * p_pwm_instance, low_power_pwm_config_t const * p_pwm_config, app_timer_timeout_handler_t handler)
{ 
    ASSERT(p_pwm_instance->pwm_state == NRF_DRV_STATE_UNINITIALIZED);
    ASSERT(p_pwm_config->bit_mask != 0);
    ASSERT(p_pwm_config->period != 0);
 
    ret_code_t err_code;
    uint32_t bit_mask;
    uint32_t pin_number = 0;
    
    p_pwm_instance->handler = handler;
    
    bit_mask = p_pwm_config->bit_mask;
    
    p_pwm_instance->active_high = p_pwm_config->active_high;
    p_pwm_instance->bit_mask = p_pwm_config->bit_mask;
    p_pwm_instance->period = p_pwm_config->period;
    p_pwm_instance->p_timer_id = p_pwm_config->p_timer_id;
    
    err_code = app_timer_create(p_pwm_instance->p_timer_id, APP_TIMER_MODE_SINGLE_SHOT, pwm_timeout_handler);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    while (bit_mask)
    {
        if (bit_mask & 0x1UL)
        {
            nrf_gpio_cfg_output(pin_number);
        }

        pin_number++;
        bit_mask >>= 1UL;
    }
    
    led_off(p_pwm_instance);
    
    p_pwm_instance->pwm_state = NRF_DRV_STATE_INITIALIZED;

    return NRF_SUCCESS;
}


ret_code_t low_power_pwm_start(low_power_pwm_t * p_pwm_instance,
                             uint32_t          leds_pin_bit_mask)
{
    ASSERT(p_pwm_instance->pwm_state == NRF_DRV_STATE_INITIALIZED);

    p_pwm_instance->evt_type = LOW_POWER_PWM_EVENT_PERIOD;
    pwm_timeout_handler(p_pwm_instance);

    p_pwm_instance->pwm_state = NRF_DRV_STATE_POWERED_ON;

    return NRF_SUCCESS;
}


ret_code_t low_power_pwm_stop(low_power_pwm_t * p_pwm_instance)
{
    ASSERT(p_pwm_instance->pwm_state == NRF_DRV_STATE_POWERED_ON);

    ret_code_t err_code;    
    
    led_off(p_pwm_instance);

    err_code = app_timer_stop(*p_pwm_instance->p_timer_id);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    p_pwm_instance->pwm_state = NRF_DRV_STATE_INITIALIZED;


    return NRF_SUCCESS;
}


ret_code_t low_power_pwm_duty_set(low_power_pwm_t * p_pwm_instance, uint8_t duty_cycle)
{
    if ( p_pwm_instance->period < duty_cycle)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    p_pwm_instance->duty_cycle = duty_cycle;

    return NRF_SUCCESS;
}
