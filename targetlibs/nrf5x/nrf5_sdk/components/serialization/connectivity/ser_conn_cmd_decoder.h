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
 * @defgroup ser_cmd_decoder Command decoder in the connectivity chip
 * @{
 * @ingroup ser_conn
 *
 * @brief   Decoder for serialized commands from an Application Chip.
 *
 * @details This file contains declaration of common function used for commands decoding and sending
 *          responses back to an Application Chip after a command is processed.
 */

#ifndef SER_CONN_CMD_DECODER_H__
#define SER_CONN_CMD_DECODER_H__

#include <stdint.h>

/**@brief A function decodes an encoded command and sends a response to an Application Chip.
 *
 * @details The function decodes an encoded command and calls a SoftDevice API function when a
 *          command decoder was found or forms a common response with error code
 *          NRF_ERROR_NOT_SUPPORTED otherwise. Then the function sends a SoftDevice response or
 *          the response with NRF_ERROR_NOT_SUPPORTED error code to an Application Chip.
 *
 * @param[in]   p_command      The encoded command.
 * @param[in]   command_len    Length of the encoded command including opcode.
 *
 * @retval    NRF_SUCCESS           Operation success.
 * @retval    NRF_ERROR_NULL        Operation failure. NULL pointer supplied.
 * @retval    NRF_ERROR_INTERNAL    Operation failure. Internal error ocurred.                                               .
 */
uint32_t ser_conn_command_process(uint8_t * p_command, uint16_t command_len);

#endif /* SER_CONN_CMD_DECODER_H__ */

/** @} */
