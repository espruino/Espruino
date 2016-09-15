#ifndef __ANT_STATE_INDICATOR_H
#define __ANT_STATE_INDICATOR_H

/** @file
 *
 * @defgroup ant_sdk_state_indicator ANT channel state indicator
 * @{
 * @ingroup ant_sdk_utils
 * @brief ANT channel state indicator module.
 *
 * @details This module provides functionality for indicating the ANT channel state.
 */

#include <stdint.h>
#include "ant_stack_handler_types.h"


/**
 * @brief Function for initializing the ANT channel state indicator.
 *
 * @details This function links the signaling procedure with a specific ANT channel.
 *
 * Before calling this function, you must initiate the @ref lib_bsp to be able to use the LEDs.
 *
 * @param[in] channel       ANT channel number.
 * @param[in] channel_type  ANT channel type (see Assign Channel Parameters in ant_parameters.h: @ref ant_parameters).
 */
void ant_state_indicator_init( uint8_t channel, uint8_t channel_type);


/**
 * @brief Function for handling ANT events.
 *
 * @details This function handles all events from the ANT stack that are of interest to the channel state indicator.
 *          This function should always be called when an ANT event occurs.
 *
 * @param[in]   p_ant_evt       Event received from the ANT stack.
 */
void ant_state_indicator_evt_handler(ant_evt_t * p_ant_evt);


/**
 * @brief Function for indicating the channel opening.
 *
 * @details This function should be called after the opening of the channel.
 *
 * @retval      NRF_SUCCESS               If the state was successfully indicated.
 * @retval      NRF_ERROR_NO_MEM          If the internal timer operations queue was full.
 * @retval      NRF_ERROR_INVALID_STATE   If the application timer module has not been initialized
 *                                        or the internal timer has not been created.
 */
uint32_t ant_state_indicator_channel_opened(void);


/**@brief Function for putting the chip into sleep mode.
 *
 * @details This function sets up a wakeup button and puts the chip into deep sleep mode.
 *
 * @note This function will not return.
 */
void ant_state_indicator_sleep_mode_enter(void);


#endif
/** @} */
