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
 * @addtogroup ser_conn Connectivity application code
 * @ingroup ble_sdk_lib_serialization
 */

/** @file
 *
 * @defgroup ser_dtm_decoder DTM Command Decoder in the connectivity chip
 * @{
 * @ingroup ser_conn
 *
 * @brief   Decoder for serialized DTM commands from an Application Chip.
 *
 * @details This file contains declaration of common function used for DTM commands decoding and
 *          sending responses back to an Application Chip after a DTM command is processed.
 */

#ifndef SER_CONN_DTM_CMD_DECODER_H__
#define SER_CONN_DTM_CMD_DECODER_H__

#include <stdint.h>

/**@brief A function for processing the encoded DTM commands from an Application Chip.
 *
 * @details     The function decodes encoded DTM commands and calls the appropriate DTM API.
 *              Then creates a DTM Command Response packet with the return value from the
 *              DTM API encoded in it and sends it to an Application Chip.
 *
 * @param[in]   p_command      The encoded command.
 * @param[in]   command_len    Length of the encoded command.
 *
 * @retval      NRF_SUCCESS    If the decoding of the command was successful, the DTM API
 *                             was called, and the command response was sent to peer, otherwise an
 *                             error code.
 */
uint32_t ser_conn_dtm_command_process(uint8_t * p_command, uint16_t command_len);


/**@brief A function for checking if Connectivity Chip is ready to enter the DTM mode.
 *
 * @details     The function checks if Connectivity Chip is ready to enter into DTM mode.
 *              If it is ready then it disables SoftDevice, closes HAL Transport Layer
 *              and starts DTM mode.
 */
void ser_conn_is_ready_to_enter_dtm(void);

#endif /* SER_CONN_DTM_CMD_DECODER_H__ */

/** @} */

