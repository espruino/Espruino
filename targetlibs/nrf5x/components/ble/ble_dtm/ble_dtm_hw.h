/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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

/** @file
 *
 * @defgroup ble_dtm_hw Direct Test Mode HW
 * @{
 * @ingroup ble_sdk_lib
 * @brief Module contains hardware related function for testing RF/PHY using DTM commands.
 */

#ifndef BLE_DTM_HW_H__
#define BLE_DTM_HW_H__

#include <stdint.h>
#include <stdbool.h>
#include "nrf.h"

#ifdef __cplusplus
extern "C" {
#endif


/**@brief Function for selecting a timer resource.
 *        This function may be called directly, or through dtm_cmd() specifying
 *        DTM_PKT_VENDORSPECIFIC as payload, SELECT_TIMER as length, and the timer as freq
 *
 * @param[out] mp_timer      Pointer to timer instance used in dtm source file.
 * @param[out] m_timer_irq   Pointer to timer interrupt related to mp_timer.
 * @param[in]  new_timer     Timer id for the timer to use.
 *
 * @retval true  if the timer was successfully changed.
 * @retval false if the error occurs.
 */

bool dtm_hw_set_timer(NRF_TIMER_Type ** mp_timer, IRQn_Type * m_timer_irq, uint32_t new_timer);


/**@brief Function for turning off radio test.
 *        This function is platform depending. For now only nRF51 requieres this special function.
 */
void dtm_turn_off_test(void);


/**@brief Function for setting constant carrier in radio settings.
 *        This function is used to handle vendor specific command testing continous carrier without
 *        a modulated signal.
 */
void dtm_constant_carrier(void);


/**@brief Function for validating tx power and radio move settings.
 * @param[in] m_tx_power    TX power for transmission test.
 * @param[in] m_radio_mode  Radio mode value.
 *
 * @retval DTM_SUCCESS                     if input parameters values are correct.
 * @retval DTM_ERROR_ILLEGAL_CONFIGURATION if input parameters values are not correct.
 */
uint32_t dtm_radio_validate(int32_t m_tx_power, uint8_t m_radio_mode);

#ifdef __cplusplus
}
#endif

#endif // BLE_DTM_HW_H__

/** @} */
