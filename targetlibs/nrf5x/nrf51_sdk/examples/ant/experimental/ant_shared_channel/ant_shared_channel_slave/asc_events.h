/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2014
All rights reserved.
*/

/**@file
 * @defgroup asc_events
 * @{
 * @ingroup ant_auto_shared_channel
 *
 * @brief Header file containing definitions of Auto Shared Channel events
 */

#ifndef ASC_EVENT_H__
#define ASC_EVENT_H__

#include <stdint.h>
#include "compiler_abstraction.h"


/**
 * @defgroup asc_events/Event Definitions
 *
 * @brief Describes the possible events that an ASC device may call
 *
 * @{
 */
#define EVENT_ASC_STATE_CHANGED         ((uint32_t)0x00000001)  /**< Event flag to indicate that the ASC device has changed its state. */
#define EVENT_ASC_DEVICE_IN_WRONG_STATE ((uint32_t)0x00000002)  /**< Event flag to indicate that an illegal attempt was made to change the state of the ASC device. */
#define EVENT_ASC_LIGHT_STATE_CHANGED   ((uint32_t)0x00000004)  /**< Event flag to indicate that the ASC device's light state has changed. */
#define EVENT_ASC_UPDATE_RECEIVED       ((uint32_t)0x00000008)  /**< Event flag to indicate that an update was received from an ASC device. */
#define EVENT_ASC_REQUEST_RECEIVED      ((uint32_t)0x00000010)  /**< Event flag to indicate that a request was received from an ASC device. */
#define EVENT_ASC_COMMAND_RECEIVED      ((uint32_t)0x00000020)  /**< Event flag to indicate that a request was received from an ASC device. */
#define EVENT_ASC_DATA_TIMEOUT          ((uint32_t)0x00000040)  /**< Event flag to indicate that the ASC device's data timeout has occurred. */
/** @} */


/** @brief Sets the ASC event flag with the event value specified.
 *
 * @param[out] p_asc_event_flags    Pointer to the flag bitfield to modify.
 *
 * @param[in]  event                The event value to set in the event flags bitfield.
 */
static __INLINE void asc_event_set(uint32_t * p_asc_event_flags, uint32_t event)
{
    *p_asc_event_flags |= event;
}


/** @brief Clears the ASC event flag with the event value specified.
 *
 * @param[out] p_asc_event_flags    Pointer to the flag bitfield to modify.
 *
 * @param[in]  event                The event value to clear from the event flags bitfield.
 */
__INLINE void asc_event_clear(uint32_t * p_asc_event_flags, uint32_t event)
{
    *p_asc_event_flags &= ~event;
}

#endif //ASC_EVENT_H__

/** @} */
