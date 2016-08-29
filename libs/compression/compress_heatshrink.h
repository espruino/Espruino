/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 *  Wrapper for heatshrink encode/decode
 * ----------------------------------------------------------------------------
 */

/** gets data from array, writes to callback */
void heatshrink_encode(unsigned char *data, size_t dataLen, void (*callback)(unsigned char ch, uint32_t *cbdata), uint32_t *cbdata);

/** gets data from callback, writes it into array */
void heatshrink_decode(int (*callback)(uint32_t *cbdata), uint32_t *cbdata, unsigned char *data);
