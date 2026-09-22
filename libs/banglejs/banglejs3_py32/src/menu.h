/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2026 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Recovery menu
 * ----------------------------------------------------------------------------
 */

/// Redraws the menu to the LCD
void menu_draw();
/// Called when state.buttonMask has changed (a button was pressed)
void menu_update();
/// Called when state.buttonMask has changed (a button was pressed)
void menu_start();
