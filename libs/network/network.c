/*
 * network.c
 *
 *  Created on: 21 Jan 2014
 *      Author: gw
 */
#include "network.h"

JsNetworkState networkState =
#ifdef USE_CC3000
    NETWORKSTATE_OFFLINE
#else
    NETWORKSTATE_ONLINE
#endif
    ;
