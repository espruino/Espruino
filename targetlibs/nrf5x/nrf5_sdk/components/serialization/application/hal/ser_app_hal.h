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
 *
 * @defgroup ser_app_hal Serialization Application Hardware Abstraction Layer
 * @{
 * @ingroup ble_sdk_lib_serialization
 *
 * @brief Functions which set up hardware on Application board and perform the reset of the Connectivity Board.
 */

#ifndef SER_APP_HAL_H_
#define SER_APP_HAL_H_

#include <stdint.h>

typedef void (*ser_app_hal_flash_op_done_handler_t)(bool success);
/**@brief Function for initializing hw modules.
 *
 * @details Function can initilize can hardware modules on application processor. It is optional to
 * implement. It is called one connectivity chip is initialized.
 *
 * @param handler Flash operation event handler
 *
 * @return @ref NRF_SUCCESS HAL initialized successfully.
 * @return @ref nrf_error "NRF_ERROR_..." HAL initialization failed.
 *
 */
uint32_t ser_app_hal_hw_init(ser_app_hal_flash_op_done_handler_t handler);

/**@brief Function for waiting for given amount of time.
 *
 * @param[in] ms Number of milliseconds to wait.
 *
 */
void ser_app_hal_delay(uint32_t ms);

/**@brief Function for clearing connectivity chip reset pin
 *
 */
void ser_app_hal_nrf_reset_pin_clear(void);

/**@brief Function for setting connectivity chip reset pin
 *
 */
void ser_app_hal_nrf_reset_pin_set(void);


/**@brief Function for setting softdevice event interrupt priority which is serving events incoming
 * from connectivity chip.
 *
 * @note Serialization solution on application side mimics SoC solution where events are handled in
 * the interrupt context in two ways: or directly in the interrupt context or message is posted to
 * the scheduler. However, it is possible that application processor is not using dedicated interrupt
 * for connectivity events. In that case this function can be left empty and
 * \ref ser_app_hal_nrf_evt_pending will directly call an interrupt handler function.
 */
void ser_app_hal_nrf_evt_irq_priority_set(void);

/**@brief Function for setting pending interrupt for serving events incoming from connectivity chip.
 *
 * @note The interrupt used for event from connectivity chip mimics behavior of SoC and it is not
 * intended to be triggered by any hardware event. This function should be the only source of
 * interrupt triggering.
 */
void ser_app_hal_nrf_evt_pending(void);


#endif /* SER_APP_HAL_H_ */
/** @} */
