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
#include "main.h"
#include "menu.h"
#include "lcd.h"
#include "mini_rtt.h"

/// Redraws the menu to the LCD
void menu_draw() {
  lcd_clear();
  lcd_print("RECOVERY MENU\r\n");
  lcd_print("-----------------\r\n");
  lcd_print((state.menuItem==0) ? " +" : "  ");
  lcd_print(" RESTART\r\n");
  lcd_print((state.menuItem==1) ? " +" : "  ");
  lcd_print(" ENTER BOOTLOADER\r\n");
  lcd_print((state.menuItem==2) ? " +" : "  ");
  lcd_print(" TURN OFF\r\n");
  lcd_print((state.menuItem==3) ? " +" : "  ");
  lcd_print(" EXIT\r\n");
  lcd_flip();
}

// Called when state.buttonMask has changed (a button was pressed)
void menu_update() {
  if (state.oldButtonMask!=0) return; // only update when
  if (state.buttonMask == 1) // BTN1
    state.menuItem = (state.menuItem+3) % 4;
  if (state.buttonMask == 2) { // BTN2
    lcd_clear();
    lcd_print("PLEASE WAIT...");
    lcd_flip();
    state.showMenu = false;
    switch (state.menuItem) {
      case 0: rtt_printf("-> Reboot\n"); nrf_reboot(); break;
      case 1: rtt_printf("-> Bootloader\n");break; // FIXME: enter bootloader
      case 2: rtt_printf("-> Off\n");break; // FIXME: turn off
      case 3: rtt_printf("-> Exit\n");
              state.input |= PY32_REDRAW_REQUEST;
              Set_State_Changed();
              break; // just exit
    }
  }
  if (state.buttonMask == 4) // BTN3
    state.menuItem = (state.menuItem+1) % 4;
  menu_draw();
}

// Called when state.buttonMask has changed (a button was pressed)
void menu_start() {
  state.showMenu = true;
  state.menuItem = 0;
  menu_draw();
}
