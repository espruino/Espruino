/*
 * network.c
 *
 *  Created on: 21 Jan 2014
 *      Author: gw
 */
#include "network.h"

JsNetworkState networkState =
#if defined(USE_CC3000) || defined(USE_WIZNET)
    NETWORKSTATE_OFFLINE
#else
    NETWORKSTATE_ONLINE
#endif
    ;
