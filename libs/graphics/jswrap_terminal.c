/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2017 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains VT100 terminal emulator
 * ----------------------------------------------------------------------------
 */
#include "jswrap_terminal.h"
#include "jswrap_graphics.h"
#include "graphics.h"
#include "jsparse.h"
#include "jsdevices.h"
#include "jshardware.h"

/*JSON{
  "type" : "object",
  "name" : "Terminal",
  "instanceof" : "Serial",
  "ifdef" : "USE_TERMINAL"
}
A simple VT100 terminal emulator.

When data is sent to the `Terminal` object, `Graphics.getInstance()` is called
and if an instance of `Graphics` is found then characters are written to it.
*/

#ifdef USE_FONT_6X8
#define TERMINAL_CHAR_W (6)
#define TERMINAL_CHAR_H (8)
#include "bitmap_font_6x8.h"
#define TERMINAL_CHAR_CMD graphicsDrawChar6x8
#else
#define TERMINAL_CHAR_W (4)
#define TERMINAL_CHAR_H (6)
#include "bitmap_font_4x6.h"
#define TERMINAL_CHAR_CMD graphicsDrawChar4x6
#endif

#ifdef LCD_HEIGHT
#define TERMINAL_HEIGHT (LCD_HEIGHT/TERMINAL_CHAR_H)
#define TERMINAL_OFFSET_Y (LCD_HEIGHT-(TERMINAL_HEIGHT*TERMINAL_CHAR_H))
#else
#define TERMINAL_HEIGHT (10)
#define TERMINAL_OFFSET_Y (4)
#endif
#define TERMINAL_OFFSET_X (0)


char terminalControlChars[4];
unsigned char terminalX = 0;
unsigned char terminalY = TERMINAL_HEIGHT-1;
bool terminalNeedsFlip = false;

static void terminalControlCharsReset() {
  terminalControlChars[0]=0;
  terminalControlChars[1]=0;
  terminalControlChars[2]=0;
  terminalControlChars[3]=0;
}

// Try and find something to use for Graphics - MUST call terminalSetGFX after if this returns true
bool terminalGetGFX(JsGraphics *gfx) {
#ifdef ESPR_GRAPHICS_INTERNAL
  // FIXME: not ideal, we should really be using the pointer directly
  if (!graphicsInternal.setPixel) return false; // not set up yet
  *gfx = graphicsInternal;
  return true;
#else
  JsVar *v = jswrap_graphics_getInstance();
  if (!v) return false;
  if (graphicsGetFromVar(gfx, v))
    return true;
  jsvUnLock(v);
  return false;
#endif
}

/// Setup the graphics var state and flip the screen
void terminalSetGFX(JsGraphics *gfx) {
#ifdef ESPR_GRAPHICS_INTERNAL
  graphicsInternal = *gfx;
#else
  graphicsSetVar(gfx);
  jsvUnLock(gfx->graphicsVar);
#endif
  terminalNeedsFlip = true; // force a flip to the screen next idle
}

/// Scroll up to leave one more line free at the bottom
void terminalScroll() {
  terminalY--;
  JsGraphics gfx;
  if (terminalGetGFX(&gfx)) {
    unsigned int cb = gfx.data.bgColor;
#ifdef GRAPHICS_THEME
    gfx.data.bgColor = graphicsTheme.bg;
#else
    gfx.data.bgColor = 0;
#endif
    graphicsScroll(&gfx, 0, -TERMINAL_CHAR_H); // always fill background in black
    gfx.data.bgColor = cb;
    terminalSetGFX(&gfx); // save
    // if we're not in an IRQ, flip this now
    if (!jshIsInInterrupt())
      jswrap_terminal_idle();
  }
}

/// Handle data sent to the VT100 terminal
void terminalSendChar(char chn) {
  if (terminalControlChars[0] == 0) {
    if (chn==8) {
      if (terminalX>0) terminalX--;
    } else if (chn==10) { // line feed
      terminalX = 0; terminalY++;
      while (terminalY >= TERMINAL_HEIGHT)
        terminalScroll();
    } else if (chn==13) { // carriage return
      terminalX = 0;
    } else if (chn==27) {
      terminalControlChars[0] = 27;
    } else if (chn==19 || chn==17) { // XOFF/XON
    } else {
      // Else actually add character
      JsGraphics gfx;
      if (terminalGetGFX(&gfx)) {
        short cx = (short)(TERMINAL_OFFSET_X + terminalX*TERMINAL_CHAR_W);
        short cy = (short)(TERMINAL_OFFSET_Y + terminalY*TERMINAL_CHAR_H + gfx.data.height - LCD_HEIGHT);
        // draw char
        unsigned int cf = gfx.data.fgColor, cb = gfx.data.bgColor;
#ifdef GRAPHICS_THEME
        gfx.data.fgColor = graphicsTheme.fg;
        gfx.data.bgColor = graphicsTheme.bg;
#else
        gfx.data.fgColor = -1; // always white on black
        gfx.data.bgColor = 0;
#endif
        TERMINAL_CHAR_CMD(&gfx, cx, cy, chn, 1, 1, true/*solid background - so no need to clear*/);
        gfx.data.fgColor = cf;
        gfx.data.bgColor = cb;
        terminalSetGFX(&gfx);
      }
      if (terminalX<255) terminalX++;
    }
  } else if (terminalControlChars[0]==27) {
    if (terminalControlChars[1]==91) {
      if (terminalControlChars[2]==63) {
        if (terminalControlChars[3]==55) {
          // if (chn!=108) jsiConsolePrintf("Expected 27, 91, 63, 55, 108 - no line overflow sequence\n");
          terminalControlCharsReset();
        } else {
          if (chn==55) {
            terminalControlChars[3] = 55;
          } else terminalControlCharsReset();
        }
      } else {
        if (chn == 63) {
          terminalControlChars[2] = 63;
        } else {
          terminalControlCharsReset();
          switch (chn) {
            case 65: if (terminalY > 0) terminalY--; break;
            case 66: terminalY++; while (terminalY >= TERMINAL_HEIGHT) terminalScroll(); break;  // down
            case 67: if (terminalX<255) terminalX++; break; // right
            case 68: if (terminalX > 0) terminalX--; break; // left
            case 74: { // delete all to right and down
              JsGraphics gfx;
              if (terminalGetGFX(&gfx)) {
                short cx = (short)(TERMINAL_OFFSET_X + terminalX*TERMINAL_CHAR_W);
                short cy = (short)(TERMINAL_OFFSET_Y + terminalY*TERMINAL_CHAR_H + gfx.data.height - LCD_HEIGHT);
                short w = (gfx.data.flags & JSGRAPHICSFLAGS_SWAP_XY) ? gfx.data.height : gfx.data.width;
                short h = (gfx.data.flags & JSGRAPHICSFLAGS_SWAP_XY) ? gfx.data.width : gfx.data.height;
                // Clear to right and down
                graphicsFillRect(&gfx, cx, cy, w-1, cy+TERMINAL_CHAR_H-1, 0/*black*/); // current line
                graphicsFillRect(&gfx, TERMINAL_OFFSET_X, cy+TERMINAL_CHAR_H, w-1, h-1, 0/*black*/); // everything under
                terminalSetGFX(&gfx);
              }
            } break;
          }
        }
      }
    } else {
      switch (chn) {
        case 91: terminalControlChars[1] = 91; break;
        default: terminalControlCharsReset(); break;
      }
    }
  } else {
    terminalControlCharsReset();
  }
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_terminal_init"
}*/
void jswrap_terminal_init() {
  terminalControlCharsReset();
  terminalX = 0;
  terminalY = (unsigned char)(TERMINAL_HEIGHT-1);
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_terminal_idle"
}*/
bool jswrap_terminal_idle() {
  if (terminalNeedsFlip) {
#ifdef ESPR_GRAPHICS_INTERNAL
    graphicsInternalFlip();
#else
    JsGraphics gfx;
    if (terminalGetGFX(&gfx)) {
      JsVar *flip = jsvObjectGetChildIfExists(gfx.graphicsVar, "flip");
      if (flip) jsvUnLock2(jspExecuteFunction(flip,gfx.graphicsVar,0,0),flip);
      jsvUnLock(gfx.graphicsVar);
    }
#endif
    terminalNeedsFlip = false;
  }
  return false;
}


