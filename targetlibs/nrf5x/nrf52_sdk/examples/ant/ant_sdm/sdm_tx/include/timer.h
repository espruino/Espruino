/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
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
 * @brief SDM TX timer interface
 * @defgroup ant_sdm_tx_timer SDM data update timer 
 * @{
 * @ingroup ant_sdm_tx_example
 *
 * @brief SDM TX timer interface
 */

#ifndef TIMER_H__
#define TIMER_H__

typedef void (*timer_callback_t)(); /**< The timer client callback prototype. */

/**@brief Power up and configure the time reference peripheral.
 */
void timer_initialize(void);

/**@brief Register a timer callback client.
 *
 * @param[in] timer_expiration_callback The timer client callback handler, 
 * which is called upon timer expiration time.
 */
void timer_register(timer_callback_t timer_expiration_callback); 

/**@brief Start the scheduling of a previously registered callback handler.
 */
void timer_start(void);

#endif // TIMER_H__

/**
 *@}
 **/
