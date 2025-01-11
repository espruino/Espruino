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
 * Graphics Backend for drawing via SDL
 * ----------------------------------------------------------------------------
 */

#include "platform_config.h"
#include "jsutils.h"
#include "lcd_sdl.h"
#include <SDL/SDL.h>

#define BPP 4
#define DEPTH 32

SDL_Surface *screen = 0;
bool needsFlip = false;

uint32_t palette_web[256] = {
    0x000000,0x000033,0x000066,0x000099,0x0000cc,0x0000ff,0x003300,0x003333,0x003366,0x003399,0x0033cc,
    0x0033ff,0x006600,0x006633,0x006666,0x006699,0x0066cc,0x0066ff,0x009900,0x009933,0x009966,0x009999,
    0x0099cc,0x0099ff,0x00cc00,0x00cc33,0x00cc66,0x00cc99,0x00cccc,0x00ccff,0x00ff00,0x00ff33,0x00ff66,
    0x00ff99,0x00ffcc,0x00ffff,0x330000,0x330033,0x330066,0x330099,0x3300cc,0x3300ff,0x333300,0x333333,
    0x333366,0x333399,0x3333cc,0x3333ff,0x336600,0x336633,0x336666,0x336699,0x3366cc,0x3366ff,0x339900,
    0x339933,0x339966,0x339999,0x3399cc,0x3399ff,0x33cc00,0x33cc33,0x33cc66,0x33cc99,0x33cccc,0x33ccff,
    0x33ff00,0x33ff33,0x33ff66,0x33ff99,0x33ffcc,0x33ffff,0x660000,0x660033,0x660066,0x660099,0x6600cc,
    0x6600ff,0x663300,0x663333,0x663366,0x663399,0x6633cc,0x6633ff,0x666600,0x666633,0x666666,0x666699,
    0x6666cc,0x6666ff,0x669900,0x669933,0x669966,0x669999,0x6699cc,0x6699ff,0x66cc00,0x66cc33,0x66cc66,
    0x66cc99,0x66cccc,0x66ccff,0x66ff00,0x66ff33,0x66ff66,0x66ff99,0x66ffcc,0x66ffff,0x990000,0x990033,
    0x990066,0x990099,0x9900cc,0x9900ff,0x993300,0x993333,0x993366,0x993399,0x9933cc,0x9933ff,0x996600,
    0x996633,0x996666,0x996699,0x9966cc,0x9966ff,0x999900,0x999933,0x999966,0x999999,0x9999cc,0x9999ff,
    0x99cc00,0x99cc33,0x99cc66,0x99cc99,0x99cccc,0x99ccff,0x99ff00,0x99ff33,0x99ff66,0x99ff99,0x99ffcc,
    0x99ffff,0xcc0000,0xcc0033,0xcc0066,0xcc0099,0xcc00cc,0xcc00ff,0xcc3300,0xcc3333,0xcc3366,0xcc3399,
    0xcc33cc,0xcc33ff,0xcc6600,0xcc6633,0xcc6666,0xcc6699,0xcc66cc,0xcc66ff,0xcc9900,0xcc9933,0xcc9966,
    0xcc9999,0xcc99cc,0xcc99ff,0xcccc00,0xcccc33,0xcccc66,0xcccc99,0xcccccc,0xccccff,0xccff00,0xccff33,
    0xccff66,0xccff99,0xccffcc,0xccffff,0xff0000,0xff0033,0xff0066,0xff0099,0xff00cc,0xff00ff,0xff3300,
    0xff3333,0xff3366,0xff3399,0xff33cc,0xff33ff,0xff6600,0xff6633,0xff6666,0xff6699,0xff66cc,0xff66ff,
    0xff9900,0xff9933,0xff9966,0xff9999,0xff99cc,0xff99ff,0xffcc00,0xffcc33,0xffcc66,0xffcc99,0xffcccc,
    0xffccff,0xffff00,0xffff33,0xffff66,0xffff99,0xffffcc,0xffffff};

unsigned int lcdGetPixel_SDL(JsGraphics *gfx, int x, int y) {
  if (!screen) return 0;
  if(SDL_MUSTLOCK(screen))
      if(SDL_LockSurface(screen) < 0) return 0;
  unsigned int *pixmem32 = ((unsigned int*)screen->pixels) + y*gfx->data.width + x;
  unsigned int col = *pixmem32;
  if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
  return col;
}


void lcdSetPixel_SDL(JsGraphics *gfx, int x, int y, unsigned int col) {
  if (!screen) return;

  if(SDL_MUSTLOCK(screen))
    if(SDL_LockSurface(screen) < 0) return;
  if (gfx->data.bpp==8) col = palette_web[col&255];
  if (gfx->data.bpp==16) {
    int r = (col>>8)&0xF8;
    int g = (col>>3)&0xFC;
    int b = (col<<3)&0xFF;
    col = (r<<16)|(g<<8)|b;
  }
  unsigned int *pixmem32 = ((unsigned int*)screen->pixels) + y*gfx->data.width + x;
  *pixmem32 = col;
  if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
  needsFlip = true;
}

void  lcdFillRect_SDL(struct JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col) {
  int x,y;
  for (y=y1;y<=y2;y++)
    for (x=x1;x<=x2;x++)
      lcdSetPixel_SDL(gfx, x, y, col);
}

void lcdInit_SDL(JsGraphics *gfx) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    jsExceptionHere(JSET_ERROR, "SDL_Init failed");
    exit(1);
  }
  if (!(screen = SDL_SetVideoMode(gfx->data.width, gfx->data.height, 32, SDL_SWSURFACE))) {
    jsExceptionHere(JSET_ERROR, "SDL_SetVideoMode failed");
    SDL_Quit();
    exit(1);
  }
  SDL_WM_SetCaption("Espruino", NULL);
}

void lcdIdle_SDL() {
  if (needsFlip) {
    needsFlip = false;
    SDL_Flip(screen);
  }
}

void lcdSetCallbacks_SDL(JsGraphics *gfx) {
  gfx->setPixel = lcdSetPixel_SDL;
  gfx->getPixel = lcdGetPixel_SDL;
  gfx->fillRect = lcdFillRect_SDL;
  // FIXME: idle callback would be a great idea to save lock/unlock
}
