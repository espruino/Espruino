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
 * @brief SDM TX module interface.
 * @defgroup ant_sdm_tx_module_interface Interface
 * @{
 * @ingroup ant_sdm_tx
 *
 * @brief SDM TX module interface.
 */

#ifndef SDM_TX_IF_H__
#define SDM_TX_IF_H__

#include <stdint.h> 

/**@brief Initialize/Reset the SDM module.
 *
 * @return ::NRF_SUCCESS.
 */
uint32_t sdm_tx_init(void);

/**@brief Get next page number to broadcast.
 *
 * @param[out] p_next_page A pointer to a variable holding the next page to be broadcast.
 *
 * @return ::NRF_SUCCESS.
 */
uint32_t sdm_tx_next_page_num_get(uint8_t * p_next_page);

/**@brief Set the broadcast data for next message.
 *
 * @param[in] page_number  The page number to prepare data for.
 * @param[out] p_message   The data buffer containing the next message.
 *
 * @return ::NRF_SUCCESS.
 * @return ::SDM_ERROR_INVALID_PAGE_NUMBER.
 */
uint32_t sdm_tx_broadcast_data_set(uint8_t page_number, uint8_t * p_message);

/**@brief Update SDM data.
 *
 * @return ::NRF_SUCCESS.
 */
uint32_t sdm_tx_sensor_data_update(void);

/**@brief Log activity on UART, GPIO or None.
 *
 * @param[in] p_message    The content of the message we want to log.
 *
 * @return ::NRF_SUCCESS
 */
uint32_t sdm_tx_log(uint8_t * p_message);

#endif // SDM_TX_IF_H__

/**
 *@}
 **/
