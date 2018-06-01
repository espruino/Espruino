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


/*JSON{
  "type" : "object",
  "name" : "Terminal",
  "instanceof" : "Serial",
  "ifdef" : "USE_TERMINAL"
}
A simple VT100 terminal emulator.

When data is sent to the `Terminal` object, `Graphics.getInstance()`
is called and if an instance of `Graphics` is found then characters
are written to it.
*/

#define terminalHeight (10)
#define terminalCharW (4)
#define terminalCharH (6)
#define terminalOffsetX (0)
#define terminalOffsetY (4)

char terminalControlChars[4];
unsigned char terminalX = 0;
unsigned char terminalY = terminalHeight-1;

static void terminalControlCharsReset() {
  terminalControlChars[0]=0;
  terminalControlChars[1]=0;
  terminalControlChars[2]=0;
  terminalControlChars[3]=0;
}

// Try and find something to use for Graphics - MUST call terminalSetGFX after if this returns true
bool terminalGetGFX(JsGraphics *gfx) {
  JsVar *v = jswrap_graphics_getInstance();
  if (!v) return false;
  if (graphicsGetFromVar(gfx, v))
    return true;
  jsvUnLock(v);
  return false;
}

// Actually display terminal contents
void terminalFlip(JsGraphics *gfx) {
  JsVar *flip = jsvObjectGetChild(gfx->graphicsVar, "flip", 0);
  if (flip) jsvUnLock2(jspExecuteFunction(flip,gfx->graphicsVar,0,0),flip);
}

/// Setup the graphics var state and flip the screen
void terminalSetGFX(JsGraphics *gfx) {
  graphicsSetVar(gfx);
  terminalFlip(gfx); // this will read from/save to graphicsVar
  jsvUnLock(gfx->graphicsVar);
}

/// Scroll up to leave one more line free at the bottom
void terminalScroll() {
  terminalY--;
  JsGraphics gfx;
  if (terminalGetGFX(&gfx)) {
    graphicsScroll(&gfx, 0, -terminalCharH);
    terminalSetGFX(&gfx); // save and flip
  }
}

/// Handle data sent to the VT100 terminal
void terminalSendChar(char chn) {
  if (terminalControlChars[0] == 0) {
    if (chn==8) {
      if (terminalX>0) terminalX--;
    } else if (chn==10) { // line feed
      terminalX = 0; terminalY++;
      while (terminalY >= terminalHeight)
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
        short cx = (short)(terminalOffsetX + terminalX*terminalCharW);
        short cy = (short)(terminalOffsetY + terminalY*terminalCharH);
        // Clear background
        unsigned int c = gfx.data.fgColor;
        gfx.data.fgColor = gfx.data.bgColor;
        graphicsFillRect(&gfx, cx, cy,
          (short)(cx+terminalCharW-1), (short)(cy+terminalCharH-1));
        gfx.data.fgColor = c;
        // draw char
        graphicsDrawChar4x6(&gfx, cx, cy, chn);
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
            case 66: terminalY++; while (terminalY >= terminalHeight) terminalScroll(); break;  // down
            case 67: if (terminalX<255) terminalX++; break; // right
            case 68: if (terminalX > 0) terminalX--; break; // left
            case 74: { // delete all to right and down
              JsGraphics gfx;
              if (terminalGetGFX(&gfx)) {
                short cx = (short)(terminalOffsetX + terminalX*terminalCharW);
                short cy = (short)(terminalOffsetY + terminalY*terminalCharH);
                short w = (gfx.data.flags & JSGRAPHICSFLAGS_SWAP_XY) ? gfx.data.height : gfx.data.width;
                short h = (gfx.data.flags & JSGRAPHICSFLAGS_SWAP_XY) ? gfx.data.width : gfx.data.height;
                // Clear to right and down
                unsigned int c = gfx.data.fgColor;
                gfx.data.fgColor = gfx.data.bgColor;
                graphicsFillRect(&gfx, cx, cy, w-1, cy+terminalCharH-1); // current line
                graphicsFillRect(&gfx, terminalOffsetX, cy+terminalCharH, w-1, h-1); // everything under
                gfx.data.fgColor = c;
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
  terminalY = (unsigned char)(terminalHeight-1);
}


