/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2013
All rights reserved.
*/

#ifndef __APP_EVENT__
#define __APP_EVENT__

#include "app_util.h"


#define EVENT_SET(event)                        \
{                                               \
    CRITICAL_REGION_ENTER();                    \
    g_event_flags|= event;                      \
    CRITICAL_REGION_EXIT();                     \
}

#define EVENT_CLEAR(event)                      \
{                                               \
    CRITICAL_REGION_ENTER();                    \
    g_event_flags&= ~event;                     \
    CRITICAL_REGION_EXIT();                     \
}




// Common timer funcs
#define APP_TIMER_PRESCALER             0                                           /**< Value of the RTC1 PRESCALER register. */

// Application Events
#define EVENT_ANT_STACK                 ((uint32_t)0x00000001)
uint8_t main_get_status(void);

#endif

