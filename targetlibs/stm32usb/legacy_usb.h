/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Hacky filler to allow old-style F1/F3 USB code to but used with the F4's 
 * new CubeMX stuff without ifdefs all over the main code
 * ----------------------------------------------------------------------------
 */

#include "usb_utils.h"
#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_pwr.h"
#include "usb_istr.h"

#include "platform_config.h"
#include "jsutils.h"

void MX_USB_DEVICE_Init();

bool USB_IsConnected();

