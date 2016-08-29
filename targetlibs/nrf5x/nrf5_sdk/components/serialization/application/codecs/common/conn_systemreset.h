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
 
#ifndef CONN_SYSTEMRESET_H__
#define CONN_SYSTEMRESET_H__

/**
 * @addtogroup ser_codecs Serialization codecs
 * @ingroup ble_sdk_lib_serialization
 */

/**
 * @addtogroup ser_app_common_codecs Application common codecs
 * @ingroup ser_codecs
 */

/**@file
 *
 * @defgroup conn_systemreset Connectivity chip reset command request encoder.
 * @{
 * @ingroup  ser_app_common_codecs
 *
 * @brief    Connectivity chip reset command request encoder.
 */

/**@brief Function for performing the connectivity chip reset.
 *
 * @retval NRF_SUCCESS          Encoding success.
 * @retval NRF_ERROR_INTERNAL   Encoding failure. Transport error.
 */
uint32_t conn_systemreset(void);

/** @} */
#endif // CONN_SYSTEMRESET_H__
