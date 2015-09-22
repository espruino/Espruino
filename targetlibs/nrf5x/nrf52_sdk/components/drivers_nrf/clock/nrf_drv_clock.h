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

#ifndef NRF_DRV_CLOCK_H__
#define NRF_DRV_CLOCK_H__

#include <stdbool.h>
#include <stdint.h>
#include "sdk_errors.h"
#include "nrf_assert.h"
#include "nrf_clock.h"
#include "nrf_drv_config.h"
#include "nrf_drv_common.h"

/**
 *
 * @addtogroup nrf_clock Clock HAL and driver
 * @ingroup nrf_drivers
 * @brief Clock APIs.
 * @details The clock HAL provides basic APIs for accessing the registers of the clock. 
 * The clock driver provides APIs on a higher level.
 *
 * @defgroup nrf_clock_drv Clock driver
 * @{
 * @ingroup nrf_clock
 * @brief Driver for managing the low-frequency clock (LFCLK) and the high-frequency clock (HFCLK).
 */

/**
 * @brief Calibration interval configuration for the low-frequency RC oscillator.
 *
 * @details  Calibration can be affected by radio transmission. To avoid this problem,
 *           call @ref nrf_drv_clock_calibration_force() and wait for end of calibration.
 */
typedef enum
{
    RC_250MS_CALIBRATION_INTERVAL   = 1,  /*< Calibration every 250 ms. */
    RC_500MS_CALIBRATION_INTERVAL   = 2,  /*< Calibration every 500 ms. */
    RC_1000MS_CALIBRATION_INTERVAL  = 4,  /*< Calibration every 1000 ms. */
    RC_2000MS_CALIBRATION_INTERVAL  = 8,  /*< Calibration every 2000 ms. */
    RC_4000MS_CALIBRATION_INTERVAL  = 16, /*< Calibration every 4000 ms. */
    RC_8000MS_CALIBRATION_INTERVAL  = 32, /*< Calibration every 8000 ms. */
    RC_16000MS_CALIBRATION_INTERVAL = 64  /*< Calibration every 16000 ms. */
} nrf_drv_clock_lf_cal_interval_t;

/**@brief Struct for Clock initialization. Thise parameters are used when SoftDevice is not present
 *        and low-frequency RC oscillator is selected.
 */
typedef struct
{
    nrf_drv_clock_lf_cal_interval_t    cal_interval;       /**< Calibration interval. */
    uint8_t                            interrupt_priority; /**< Clock interrupt priority. */
} nrf_drv_clock_config_t;

/**@brief Clock default configuration.*/
#define NRF_DRV_CLOCK_DEAFULT_CONFIG                             \
    {                                                            \
        .cal_interval       = CLOCK_CONFIG_LF_RC_CAL_INTERVAL,   \
        .interrupt_priority = CLOCK_CONFIG_IRQ_PRIORITY,         \
    }

/**
 * @brief Function for initialization the nrf_drv_clock module.
 *
 * After initialization, the module is in power off state (clocks are not requested).
 *
 * @param[in]  p_config                           Initial configuration. Default configuration used if NULL.
 *
 * @retval     NRF_SUCCESS                        If the procedure was successful.
 * @retval     MODULE_ALREADY_INITIALIZED         If the driver was already initialized.
 * @retval     NRF_ERROR_SOFTDEVICE_NOT_ENABLED   If the SoftDevice was not enabled.
 */
ret_code_t nrf_drv_clock_init(nrf_drv_clock_config_t const * p_config);

/**
 * @brief Function for uninitialization the nrf_drv_clock module.
 *
 * After uninitialization, the module is in idle state.
 */
void nrf_drv_clock_uninit(void);

/**
 * @brief Function for requesting LFCLK. LFCLK can be repeatedly requested.
 *
 * @note When Softdevice is enabled, LFCLK is always running.
 *
 * @details If it is first request, selected LFCLK source will be started. 
 *          The @ref nrf_drv_clock_lfclk_is_running() function can be polled to check if it has started.
 */
void nrf_drv_clock_lfclk_request(void);

/**
 * @brief Function for releasing LFCLK. If there is no more requests, LFCLK source will be stopped.
 *
 * @note When Softdevice is enabled, LFCLK is always running.
 */
void nrf_drv_clock_lfclk_release(void);

/**
 * @brief Function for checking LFCLK state.
 *
 * @retval true if the LFCLK is running, false if not.
 */
bool nrf_drv_clock_lfclk_is_running(void);

/**
 * @brief Function for requesting high-accuracy (for \nRFXX it is XTAL) source HFCLK. High-accuracy source 
 *        can be repeatedly requested.
 *
 * @details The @ref nrf_drv_clock_hfclk_is_running() function can be polled to check if it has started.
 */
void nrf_drv_clock_hfclk_request(void);

/**
 * @brief Function for releasing high-accuracy source HFCLK. If there is no more requests, high-accuracy 
 *        source will be released.
 */
void nrf_drv_clock_hfclk_release(void);

/**
 * @brief Function for checking HFCLK state.
 *
 * @retval true if the HFCLK is running (for \nRFXX XTAL source), false if not.
 */
bool nrf_drv_clock_hfclk_is_running(void);

/**
 * @brief Function for forcing calibration. 
 *
 * @details This function resets the calibration interval timer. The @ref nrf_drv_clock_is_calibrating() 
 *          function can be polled to check if calibration is still in progress.
 *
 * @retval     NRF_SUCCESS                        If the procedure was successful.
 * @retval     NRF_ERROR_FORBIDDEN                If Softdevice is present or selected LFCLK source is not RC oscillator.
 */
ret_code_t nrf_drv_clock_calibration_force(void);

/**
 * @brief Function for checking if calibration is in progress. 
 *
 * @param[out] p_is_calibrating                   true if calibration is in progress, false if not.
 *
 * @retval     NRF_SUCCESS                        If the procedure was successful.
 * @retval     NRF_ERROR_FORBIDDEN                If Softdevice is present or selected LFCLK source is not RC oscillator.
 */
ret_code_t nrf_drv_clock_is_calibrating(bool * p_is_calibrating);

/**@brief Function for returning a requested task address for the clock driver module.
 *
 * @param[in]  task                               One of the peripheral tasks.
 *
 * @retval     Task address.
 */
__STATIC_INLINE uint32_t nrf_drv_clock_ppi_task_addr(nrf_clock_task_t task)
{
    return nrf_clock_task_address_get(task);
}

/**@brief Function for returning a requested event address for the clock driver module.
 *
 * @param[in]  event                              One of the peripheral events.
 *
 * @retval     Event address.
 */
__STATIC_INLINE uint32_t nrf_drv_clock_ppi_event_addr(nrf_clock_event_t event)
{
    return nrf_clock_event_address_get(event);
}
/**
 *@}
 **/

/*lint --flb "Leave library region" */
#endif // NRF_CLOCK_H__
