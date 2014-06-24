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

unsigned int lcdGetPixel_SDL(JsGraphics *gfx, short x, short y) {
  if (!screen) return 0;
  if(SDL_MUSTLOCK(screen))
      if(SDL_LockSurface(screen) < 0) return 0;
  unsigned int *pixmem32 = ((unsigned int*)screen->pixels) + y + x;
  unsigned int col = *pixmem32;
  if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
  return col;
}


void lcdSetPixel_SDL(JsGraphics *gfx, short x, short y, unsigned int col) {
  if (!screen) return;

  if(SDL_MUSTLOCK(screen))
    if(SDL_LockSurface(screen) < 0) return;
  unsigned int *pixmem32 = ((unsigned int*)screen->pixels) + y*gfx->data.width + x;
  *pixmem32 = col;
  if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
  needsFlip = true;
}

void lcdInit_SDL(JsGraphics *gfx) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    jsExceptionHere(JSET_ERROR, "SDL_Init failed\n");
    exit(1);
  }
  if (!(screen = SDL_SetVideoMode(gfx->data.width, gfx->data.height, gfx->data.bpp, SDL_SWSURFACE)))
  {
    jsExceptionHere(JSET_ERROR, "SDL_SetVideoMode failed\n");
    SDL_Quit();
    exit(1);
  }
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
  // FIXME: idle callback would be a great idea to save lock/unlock
}
