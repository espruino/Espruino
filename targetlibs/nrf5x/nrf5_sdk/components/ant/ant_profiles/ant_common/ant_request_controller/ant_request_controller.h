/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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
 * @defgroup ant_request_controller ANT request controller
 * @{
 * @ingroup ant_sdk_utils
 *
 * @brief   Module for handling page requests related to page 70.
 */

#ifndef ANT_REQUEST_CONTROLLER_H__
#define ANT_REQUEST_CONTROLLER_H__
#include <stdbool.h>
#include "ant_common_page_70.h"
#include "ant_stack_handler_types.h"

/**@brief Request controller events types. */
typedef enum
{
    ANT_REQUEST_CONTROLLER_NONE,    ///< No event.
    ANT_REQUEST_CONTROLLER_SUCCESS, ///< Page request successful.
    ANT_REQUEST_CONTROLLER_FAILED,  ///< Page request failed.
} ant_request_controller_evt_t;

/**@brief Request controller states. */
typedef enum
{
    ANT_REQUEST_CONTROLLER_IDLE,                        ///< Module is in idle state.
    ANT_REQUEST_CONTROLLER_SENDED,                      ///< Module waits for acknowledgment of its request.
    ANT_REQUEST_CONTROLLER_BROADCAST_REQUESTED,         ///< Module is requested to send page n times using broadcast.
    ANT_REQUEST_CONTROLLER_ACK_REQUESTED,               ///< Module is requested to send page n times using acknowledgment.
    ANT_REQUEST_CONTROLLER_ACK_UNTIL_SUCCESS_REQUESTED, ///< Module is requested to send page until success using acknowledgment.
} ant_request_controller_state_t;

/**@brief ANT request controller structure. */
typedef struct
{
    ant_request_controller_state_t state;   ///< Actual module state.
    ant_common_page70_data_t       page_70; ///< Page 70.
} ant_request_controller_t;

/**@brief Function for initializing the ANT request controller instance.
 *
 * @param[in]  p_controller     Pointer to the controller instance.
 */
void ant_request_controller_init(ant_request_controller_t * p_controller);

/**@brief Function for sending a request.
 *
 * @param[in]  p_controller     Pointer to the controller instance.
 * @param[in]  channel_number   Channel number.
 * @param[in]  p_page_70        Pointer to the prepared page 70.
 *
 * @return     Error code returned by @ref sd_ant_acknowledge_message_tx().
 */
uint32_t ant_request_controller_request(ant_request_controller_t * p_controller,
                                        uint8_t                    channel_number,
                                        ant_common_page70_data_t * p_page_70);

/**@brief Function for getting pending page number.
 *
 * @details This function checks whether a page number was requested.
 *
 * @param[in]  p_controller     Pointer to the controller instance.
 * @param[out] p_page_number    Pending page number (valid if true was returned).
 *
 * @retval     TRUE             If there was a pending page.
 * @retval     FALSE            If no page was pending.
 */
bool ant_request_controller_pending_get(ant_request_controller_t * p_controller,
                                        uint8_t                  * p_page_number);

/**@brief Function for checking whether the next page must be sent with acknowledgment.
 *
 * @param[in]  p_controller     Pointer to the controller instance.
 *
 * @retval     TRUE             If the next transmission needs acknowledgment.
 * @retval     FALSE            If the next transmission does not need acknowledgment.
 */
bool ant_request_controller_ack_needed(ant_request_controller_t * p_controller);

/**
 * @brief Function for handling ANT events on display side.
 *
 * @details All events from the ANT stack that are related to the appropriate channel number
 *          should be propagated.
 *
 * @param[in]  p_controller             Pointer to the controller instance.
 * @param[in]  p_ant_event              Event received from the ANT stack.
 */
ant_request_controller_evt_t ant_request_controller_disp_evt_handler(
    ant_request_controller_t * p_controller,
    ant_evt_t                * p_ant_event);

/**
 * @brief Function for handling ANT events on sensor side.
 *
 * @details All events from the ANT stack that are related to the appropriate channel number
 *          should be propagated.
 *
 * @param[in]  p_controller             Pointer to the controller instance.
 * @param[in]  p_ant_event              Event received from the ANT stack.
 * @retval     TRUE                     If there was a pending page.
 */
void ant_request_controller_sens_evt_handler(ant_request_controller_t * p_controller,
                                            ant_evt_t                * p_ant_event);


#endif // ANT_REQUEST_CONTROLLER_H__
/** @} */
