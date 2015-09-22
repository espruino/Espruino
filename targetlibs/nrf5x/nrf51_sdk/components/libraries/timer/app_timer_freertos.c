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

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "app_timer.h"
#include <stdlib.h>
#include "nrf51.h"
#include "nrf51_bitfields.h"
#include "app_error.h"

portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

/**@brief Creates a FreeRTOS Timer
 *
 * @param[in]  p_timer_id        Id of created timer.
 * @param[in]  mode              Mode of created timer.
 * @param[in]  timeout_handler   Function which will be executed when timer expires.
 *
 * @return     This function returns timer handle (ID).
 */
uint32_t app_timer_create(app_timer_id_t            * p_timer_id,
                          app_timer_mode_t            mode,
                          app_timer_timeout_handler_t timeout_handler)
{
    uint32_t      err_code;
    unsigned long timer_mode;

    if ( mode == APP_TIMER_MODE_SINGLE_SHOT )
        timer_mode = pdFALSE;
    else
        timer_mode = pdTRUE;

    *p_timer_id = xTimerCreate(" ", 1000, timer_mode, NULL, timeout_handler);

    if( p_timer_id != NULL )
        err_code = NRF_SUCCESS;
    else
        err_code = NRF_ERROR_NULL;

    return err_code;
}


/**@brief Start a FreeRTOS timer.
 *
 * @param[in]  p_timer_id        Id of timer.
 * @param[in]  timeout_ticks     The timer period in [ms].
 * @param[in]  p_contex          This pointer should be always NULL.
 *
 * @return     NRF_SUCCESS on success, otherwise error code.
 */
uint32_t app_timer_start(TimerHandle_t timer_id, uint32_t timeout_ticks, void * p_context)
{
    if (__get_IPSR() != 0)
    {
        if( xTimerChangePeriodFromISR( timer_id, timeout_ticks, 
                                       &xHigherPriorityTaskWoken) != pdPASS )
        {
            if( xTimerStartFromISR( timer_id, &xHigherPriorityTaskWoken ) != pdPASS )
                return NRF_ERROR_NOT_FOUND;
        }
        else
            return NRF_SUCCESS;
    }
    else
    {
        xTimerChangePeriod(timer_id, timeout_ticks, NULL);

        if( xTimerStart(timer_id, NULL) != pdPASS )
            return NRF_ERROR_NOT_FOUND;
        else
            return NRF_SUCCESS;
    }
    return NRF_ERROR_NOT_FOUND;
}

/**@brief Stop a FreeRTOS timer.
 *
 * @param[in]  p_timer_id        Id of timer.
 *
 * @return     NRF_SUCCESS on success, otherwise error code.
 */
uint32_t app_timer_stop(TimerHandle_t timer_id)
{
    if( xTimerStop(timer_id, NULL) != pdPASS )
        return NRF_ERROR_NOT_FOUND;
    else
        return NRF_SUCCESS;
}


