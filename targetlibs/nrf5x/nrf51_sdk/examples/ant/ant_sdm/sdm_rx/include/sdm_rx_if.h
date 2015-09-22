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
 * @brief SDM RX module interface.
 * @defgroup ant_sdm_rx_module_interface Interface
 * @{
 * @ingroup ant_sdm_rx
 *
 * @brief SDM RX module interface.
 */

#ifndef SDM_RX_IF_H__
#define SDM_RX_IF_H__

#include <stdint.h> 

/**@brief Function for initializing and resetting the SDM module.
 */
uint32_t sdm_rx_init(void);

/**@brief Function for processing the received data. Figure out the page received and save data to 
 *        corresponding buffers.
 *
 * @param[in] p_page_data The SDM page data to process.
 *
 * @return ::NRF_SUCCESS.
 */
uint32_t sdm_rx_data_process(uint8_t * p_page_data);

/**@brief Function for logging activity on UART, GPIO or None.
 *
 * @param[in] page        The page number to write log for.
 *
 * @return ::NRF_SUCCESS.
 */
uint32_t sdm_rx_log(uint8_t page);

#endif // SDM_RX_IF_H__

/**
 *@}
 **/
