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
 * @defgroup ser_sd_transport Serialization SoftDevice Transport
 * @{
 * @ingroup ser_app
 *
 * @brief   Serialization SoftDevice Transport on application side.
 *
 * @details This file contains declarations of functions and definitions of data structures and
 *          identifiers (typedef enum) used as API of the serialization of SoftDevice. This layer
 *          ensures atomic nature of SoftDevice calls (command and waiting for response). Packet
 *          type field of incoming packets is handled in this layer - responses are handled by
 *          ser_sd_transport (using response decoder handler provided for each SoftDevice call) but
 *          events are forwarded to the user so it is user's responsibility to free RX buffer.
 *
 */
#ifndef SER_SD_TRANSPORT_H_
#define SER_SD_TRANSPORT_H_

#include <stdint.h>
#include <stdbool.h>

typedef void (*ser_sd_transport_evt_handler_t)(uint8_t * p_buffer, uint16_t length);
typedef void (*ser_sd_transport_rsp_wait_handler_t)(void);
typedef void (*ser_sd_transport_rsp_set_handler_t)(void);
typedef void (*ser_sd_transport_rx_notification_handler_t)(void);

typedef uint32_t (*ser_sd_transport_rsp_handler_t)(const uint8_t * p_buffer, uint16_t length);

/**@brief Function for opening the module.
 *
 * @note 'Wait for response' and 'Response set' callbacks can be set in RTOS environment.
 *       It enables rescheduling while waiting for connectivity chip response. In nonOS environment
 *       usually 'Wait for response' will only be used for handling incoming events or force
 *       application to low power mode.
 *
 * @param[in] evt_handler               Handler to be called when event packet is received.
 * @param[in] os_rsp_wait_handler       Handler to be called after request is send. It should.
 *                                      implement 'Wait for signal' functionality in OS environment.
 * @param[in] os_rsp_set_handler        Handler to be called after response reception. It should
 *                                      implement 'Signal Set' functionality in OS environment
 * @param[in] rx_not_handler            Handler to be called after transport layer notifies that
 *                                      there is incoming rx packet detected.
 *
 * @retval NRF_SUCCESS              Operation success.
 * @retval NRF_ERROR_NULL           Operation failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_PARAM  Operation failure. Parameter propagated from ser_hal_transport
 *                                  opening or timer creation.
 * @retval NRF_ERROR_INVALID_STATE  Operation failure. Parameter propagated from ser_hal_transport
 *                                  opening or timer creation.
 * @retval NRF_ERROR_INTERNAL       Operation failure.  Parameter propagated from ser_hal_transport
 *                                  opening or timer creation.
 * @retval NRF_ERROR_NO_MEM         Operation failure.  Parameter propagated from timer creation.
 */
uint32_t ser_sd_transport_open(ser_sd_transport_evt_handler_t             evt_handler,
                               ser_sd_transport_rsp_wait_handler_t        os_rsp_wait_handler,
                               ser_sd_transport_rsp_set_handler_t         os_rsp_set_handler,
                               ser_sd_transport_rx_notification_handler_t rx_not_handler);

/**@brief Function setting 'One Time' handler to be called between sending next request packet and
 *        receiving response packet.
 * @note It is intended to be used in nonOS environment to implement concurrency.
 * @note It is 'One Time' handler meaning that it is valid only for next softdevice call processing.
 *
 *
 * @param[in] wait_handler       Handler to be called after request packet is sent.
 *
 * @retval NRF_SUCCESS          Operation success.
 */
uint32_t ser_sd_transport_ot_rsp_wait_handler_set(ser_sd_transport_rsp_wait_handler_t wait_handler);


/**@brief Function for closing the module.
 *
 * @retval NRF_SUCCESS          Operation success.
 */
uint32_t ser_sd_transport_close(void);

/**@brief Function for allocating tx packet to be used for request command.
 *
 * @param[out] pp_data       Pointer to data pointer to be set to point to allocated buffer.
 * @param[out] p_len         Pointer to allocated buffer length.
 *
 * @retval NRF_SUCCESS          Operation success.
 */
uint32_t ser_sd_transport_tx_alloc(uint8_t * * pp_data, uint16_t * p_len);


/**@brief Function for freeing tx packet.
 *
 * @note Function should be called once command is processed.
 *
 * @param[out] p_data       Pointer to allocated tx buffer.
 *
 * @retval NRF_SUCCESS          Operation success.
 */
uint32_t ser_sd_transport_tx_free(uint8_t * p_data);


/**@brief Function for freeing RX event packet.
 *
 * @note Function should be called once SoftDevice event buffer is processed.
 *
 * @param[out] p_data       Pointer to allocated rx buffer.
 *
 * @retval NRF_SUCCESS          Operation success.
 */
uint32_t ser_sd_transport_rx_free(uint8_t * p_data);


/**@brief Function for checking if module is busy waiting for response from connectivity side.
 *
 * @retval true      Module busy. Cannot accept next command.
 * @retval false     Module not busy. Can accept next command.
 */
bool ser_sd_transport_is_busy(void);

/**@brief Function for handling SoftDevice command.
 *
 * @note Function blocks task context until response is received and processed.
 * @note Non-blocking functionality can be achieved using os handlers or 'One Time' handler
 * @warning Function shouldn't be called from interrupt context which would block switching to
 *          serial port interrupt.
 *
 * @param[in] p_buffer                 Pointer to command.
 * @param[in] length                   Pointer to allocated buffer length.
 * @param[in] cmd_resp_decode_callback Pointer to function for decoding response packet.
 *
 * @retval NRF_SUCCESS          Operation success.
 */
uint32_t ser_sd_transport_cmd_write(const uint8_t *                p_buffer,
                                    uint16_t                       length,
                                    ser_sd_transport_rsp_handler_t cmd_resp_decode_callback);

#endif /* SER_SD_TRANSPORT_H_ */
/** @} */
