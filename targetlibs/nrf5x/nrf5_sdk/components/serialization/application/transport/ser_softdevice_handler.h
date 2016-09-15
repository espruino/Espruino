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

/**
 * @addtogroup ser_app Application side code
 * @ingroup ble_sdk_lib_serialization
 */

/** @file
 *
 * @defgroup ser_softdevice_handler Serialization SoftDevice Handler
 * @{
 * @ingroup ser_app
 *
 * @brief   Serialization SoftDevice Handler on application side.
 *
 */
#ifndef SER_SOFTDEVICE_HANDLER_H_
#define SER_SOFTDEVICE_HANDLER_H_

#include <stdint.h>
#include <stdbool.h>


/**@brief Function for checking if there is any more events in the internal mailbox.
 *
 * @param[in] p_mailbox_length Pointer to mailbox length.
 *
 * @retval ::NRF_SUCCESS    Length succesfully obtained.
 * @retval ::NRF_ERROR_NULL Null pointer provided.
 */
uint32_t sd_ble_evt_mailbox_length_get(uint32_t * p_mailbox_length);

#endif /* SER_SOFTDEVICE_HANDLER_H_ */
/** @} */
