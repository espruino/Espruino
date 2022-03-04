/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2021 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Step Counter
 * ----------------------------------------------------------------------------
 */

/// Initialise step counting
void stepcount_init();

/* Registers a new data point for step counting. Data is expected
 * as 12.5Hz, 8192=1g, and accMagSquared = x*x + y*y + z*z
 *
 * Returns the number of steps counted for this accel interval
 */
int stepcount_new(int accMagSquared);
