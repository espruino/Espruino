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

/**@file
 * @addtogroup nrf_wdt WDT HAL and driver
 * @ingroup nrf_drivers
 * @defgroup lib_driver_wdt WDT driver
 * @{
 * @ingroup  nrf_wdt
 *
 * @brief    WDT driver.
 */

#ifndef NRF_DRV_WDT_H__
#define NRF_DRV_WDT_H__

#include <stdbool.h>
#include <stdint.h>
#include "sdk_errors.h"
#include "nrf_wdt.h"
#include "nrf_drv_config.h"

/**@brief Struct for WDT initialization. */
typedef struct
{
    nrf_wdt_behaviour_t    behaviour;          /**< WDT behaviour when CPU in sleep/halt mode. */
    uint32_t               reload_value;       /**< WDT reload value in ms. */
    uint8_t                interrupt_priority; /**< WDT interrupt priority */
} nrf_drv_wdt_config_t;

/**@brief WDT event handler function type. */
typedef void (*nrf_wdt_event_handler_t)(void);

/**@brief WDT channel id type. */
typedef nrf_wdt_rr_register_t nrf_drv_wdt_channel_id;

#define NRF_DRV_WDT_DEAFULT_CONFIG                     \
    {                                                  \
        .behaviour          = WDT_CONFIG_BEHAVIOUR,    \
        .reload_value       = WDT_CONFIG_RELOAD_VALUE, \
        .interrupt_priority = WDT_CONFIG_IRQ_PRIORITY, \
    }
/**
 * @brief This function initializes watchdog.
 *
 * @param[in] p_config          Initial configuration. Default configuration used if NULL.
 * @param[in] wdt_event_handler specifies event handler provided by user.
 *
 * @note Function asserts if wdt_event_handler is NULL.
 *
 * @return    NRF_SUCCESS on success, NRF_ERROR_INVALID_STATE if module ws already initialized.
 */
ret_code_t nrf_drv_wdt_init(nrf_drv_wdt_config_t const * p_config,
                            nrf_wdt_event_handler_t     wdt_event_handler);

/**
 * @brief This function allocate watchdog channel.
 *
 * @note This function can not be called after nrf_drv_wdt_start(void).
 *
 * @param[out] p_channel_id      ID of granted channel.
 *
 * @return    NRF_SUCCESS on success, otherwise an error code.
 */
ret_code_t nrf_drv_wdt_channel_alloc(nrf_drv_wdt_channel_id * p_channel_id);

/**
 * @brief This function starts watchdog.
 *
 * @note After calling this function the watchdog is started, so the user needs to feed all allocated
 *       watchdog channels to avoid reset. At least one watchdog channel has to be allocated.
 */
void nrf_drv_wdt_enable(void);

/**
 * @brief This function feeds the watchdog.
 *
 * @details Function feeds all allocated watchdog channels.
 */
void nrf_drv_wdt_feed(void);

/**
 * @brief This function feeds the invidual watchdog channel.
 *
 * @param[in] channel_id      ID of watchdog channel.
 */
void nrf_drv_wdt_channel_feed(nrf_drv_wdt_channel_id channel_id);

/**@brief Function for returning a requested task address for the wdt driver module.
 *
 * @param[in]  task                One of the peripheral tasks.
 *
 * @retval     Task address.
 */
__STATIC_INLINE uint32_t nrf_drv_wdt_ppi_task_addr(nrf_wdt_task_t task)
{
    return nrf_wdt_task_address_get(task);
}

/**@brief Function for returning a requested event address for the wdt driver module.
 *
 * @param[in]  event               One of the peripheral events.
 *
 * @retval     Event address
 */
__STATIC_INLINE uint32_t nrf_drv_wdt_ppi_event_addr(nrf_wdt_event_t event)
{
    return nrf_wdt_event_address_get(event);
}
#endif

/** @} */
