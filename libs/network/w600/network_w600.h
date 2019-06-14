/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2019 BeanJS
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains W600 board network specific function definitions.
 * ----------------------------------------------------------------------------
 */

#ifndef __NETWORK_W600_H__
#define __NETWORK_W600_H__

#include "network.h"

void netSetCallbacks_w600(JsNetwork *net);

#endif /* __NETWORK_W600_H__ */