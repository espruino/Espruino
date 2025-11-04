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
#include "jsparse.h"
#include "jsinteractive.h"
#include "lcd_sdl.h"
#include <SDL.h>

#define BPP 4
#define DEPTH 32

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL; // holds uploaded framebuffer for rendering
// 'screen' is our offscreen framebuffer at logical (gfx) size
SDL_Surface *screen = NULL;
int fbW = 0, fbH = 0; // logical framebuffer size
int winW = 0, winH = 0; // current window (render) size
bool needsFlip = false;

// Compute a destination rect that preserves fb aspect ratio within the window
static void compute_draw_rect(int winW, int winH, int fbW, int fbH, SDL_Rect *dst) {
  if (!dst) return;
  if (winW <= 0 || winH <= 0) {
    dst->x = 0; dst->y = 0; dst->w = 0; dst->h = 0; return;
  }
  if (fbW <= 0 || fbH <= 0) {
    dst->x = 0; dst->y = 0; dst->w = winW; dst->h = winH; return;
  }
  // Compare winW/fbW vs winH/fbH by cross-multiplying to avoid floats
  long long w = winW, h = winH, fw = fbW, fh = fbH;
  int drawW, drawH;
  if (w * fh > h * fw) {
    // window is wider -> fit by height
    drawH = (int)h;
    drawW = (int)((h * fw) / fh);
  } else {
    // window is taller -> fit by width
    drawW = (int)w;
    drawH = (int)((w * fh) / fw);
  }
  dst->w = drawW;
  dst->h = drawH;
  dst->x = (winW - drawW) / 2;
  dst->y = (winH - drawH) / 2;
}

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
  int stride = screen->pitch / 4; // 32-bit pixels
  unsigned int *pixmem32 = ((unsigned int*)screen->pixels) + y*stride + x;
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
    unsigned int r = (col>>8)&0xF8;
    unsigned int g = (col>>3)&0xFC;
    unsigned int b = (col<<3)&0xFF;
    col = (r<<16)|(g<<8)|b;
  }
  // Ensure alpha channel is fully opaque for ARGB8888
  col |= 0xFF000000u;
  int stride = screen->pitch / 4; // 32-bit pixels
  unsigned int *pixmem32 = ((unsigned int*)screen->pixels) + y*stride + x;
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
  // Help Linux WMs/Wayland display the app name/title correctly
  SDL_SetHint(SDL_HINT_APP_NAME, "Espruino");
  window = SDL_CreateWindow(
      "Espruino",
      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      gfx->data.width, gfx->data.height,
      SDL_WINDOW_RESIZABLE
  );
  if (!window) {
    jsExceptionHere(JSET_ERROR, "SDL_CreateWindow failed");
    SDL_Quit();
    exit(1);
  }
  // Ensure title is applied immediately and window is shown
  SDL_SetWindowTitle(window, "Espruino");
  SDL_ShowWindow(window);
  // Create renderer (accelerated if possible, fallback to software)
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!renderer) renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  if (!renderer) {
    jsExceptionHere(JSET_ERROR, "SDL_CreateRenderer failed");
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(1);
  }
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest"); // keep pixels crisp
  if (SDL_GetRendererOutputSize(renderer, &winW, &winH) != 0) {
    SDL_GetWindowSize(window, &winW, &winH);
  }
  fbW = gfx->data.width;
  fbH = gfx->data.height;
  // Create offscreen framebuffer in a known pixel format
  screen = SDL_CreateRGBSurfaceWithFormat(0, fbW, fbH, 32, SDL_PIXELFORMAT_ARGB8888);
  if (!screen) {
    jsExceptionHere(JSET_ERROR, "SDL_CreateRGBSurfaceWithFormat failed");
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
    exit(1);
  }
    SDL_SetSurfaceBlendMode(screen, SDL_BLENDMODE_NONE);
  // Create texture to upload the framebuffer each frame
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, fbW, fbH);
  if (!texture) {
    jsExceptionHere(JSET_ERROR, "SDL_CreateTexture failed");
    if (screen) SDL_FreeSurface(screen);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
    exit(1);
  }
  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
  // Force an initial present
  needsFlip = true;
}

void lcdIdle_SDL() {
  static bool down;
  extern void nativeQuit();
  SDL_Event event;
  bool sendTouchEvent = false;
  char *sendKeyEvent = NULL;
  static int mouseX = 0, mouseY = 0;

  if (needsFlip) {
    needsFlip = false;
    if (renderer && texture && screen) {
      // Upload framebuffer contents to texture
      SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
      SDL_RenderClear(renderer);
      SDL_Rect dst;
      compute_draw_rect(winW, winH, fbW, fbH, &dst);
      SDL_RenderCopy(renderer, texture, NULL, &dst);
      SDL_RenderPresent(renderer);
    }
  }

  if (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        nativeQuit();
        break;
      case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
          if (renderer && SDL_GetRendererOutputSize(renderer, &winW, &winH) != 0) {
            SDL_GetWindowSize(window, &winW, &winH);
          }
          needsFlip = true;
        } else if (event.window.event == SDL_WINDOWEVENT_EXPOSED) {
          /* Some drivers/platforms don't send RESIZED/SIZE_CHANGED when window
             is uncovered; EXPOSED indicates the window needs redraw. */
          needsFlip = true;
        }
        break;
      case SDL_MOUSEMOTION:
        mouseX = event.motion.x;
        mouseY = event.motion.y;
        if (down) sendTouchEvent = true;
        break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        down = event.type == SDL_MOUSEBUTTONDOWN;
        mouseX = event.button.x;
        mouseY = event.button.y;
        sendTouchEvent = true;
	      break;
      case SDL_KEYDOWN:
        sendKeyEvent = JS_EVENT_PREFIX"keydown";
        switch (event.key.keysym.sym) {
          case SDLK_ESCAPE: nativeQuit(); break;
          default:break;
        }
        break;
      case SDL_KEYUP:
        sendKeyEvent = JS_EVENT_PREFIX"keyup";
        break;
    }
  }

  if (sendTouchEvent) {
    JsVar *E = jsvObjectGetChildIfExists(execInfo.root, "E");
    if (E) {
      JsVar *o = jsvNewObject();
      int lx = mouseX, ly = mouseY;
      if (fbW > 0 && fbH > 0 && winW > 0 && winH > 0) {
        SDL_Rect dst;
        compute_draw_rect(winW, winH, fbW, fbH, &dst);
        long long rx = (long long)(mouseX - dst.x);
        long long ry = (long long)(mouseY - dst.y);
        // Scale from draw area back to framebuffer pixels
        if (dst.w > 0) lx = (int)(rx * fbW / dst.w); else lx = 0;
        if (dst.h > 0) ly = (int)(ry * fbH / dst.h); else ly = 0;
        if (lx < 0) lx = 0; 
        if (lx >= fbW) lx = fbW-1;
        if (ly < 0) ly = 0; 
        if (ly >= fbH) ly = fbH-1;
      }
      jsvObjectSetChildAndUnLock(o,"x", jsvNewFromInteger(lx));
      jsvObjectSetChildAndUnLock(o,"y", jsvNewFromInteger(ly));
      jsvObjectSetChildAndUnLock(o,"b", jsvNewFromInteger(down?1:0));
      jsiQueueObjectCallbacks(E, JS_EVENT_PREFIX"touch", &o, 1);
      jsvUnLock2(E,o);
    }
  }
  if (sendKeyEvent) {
    JsVar *E = jsvObjectGetChildIfExists(execInfo.root, "E");
    if (E) {
      JsVar *o = jsvNewObject();
      /* Backwards compatibility: historically (SDL 1.2) keyCode exposed the
         keysym value. Preserve that behavior so existing code keeps working.
         Also provide the scancode separately as `scanCode` for clients that
         need the physical key position. */
      jsvObjectSetChildAndUnLock(o,"keyCode", jsvNewFromInteger(event.key.keysym.sym));
      jsvObjectSetChildAndUnLock(o,"scanCode", jsvNewFromInteger(event.key.keysym.scancode));
      jsvObjectSetChildAndUnLock(o,"modifiers", jsvNewFromInteger(event.key.keysym.mod));
      const char *name = NULL;
      char tmpkey[2] = {0,0};
      int sym = event.key.keysym.sym;
      if (sym >= SDLK_a && sym <= SDLK_z) {
        tmpkey[0] = (char)('a' + (sym - SDLK_a));
        name = tmpkey;
      } else if (sym >= SDLK_0 && sym <= SDLK_9) {
        tmpkey[0] = (char)('0' + (sym - SDLK_0));
        name = tmpkey;
      } else {
        switch (sym) {
          case SDLK_SPACE: name = "Space"; break;
          case SDLK_UP: name = "ArrowUp"; break;
          case SDLK_DOWN: name = "ArrowDown"; break;
          case SDLK_LEFT: name = "ArrowLeft"; break;
          case SDLK_RIGHT: name = "ArrowRight"; break;
          case SDLK_RETURN: name = "Enter"; break;
          case SDLK_BACKSPACE: name = "Backspace"; break;
          case SDLK_TAB: name = "Tab"; break;
          case SDLK_DELETE: name = "Delete"; break;
          case SDLK_HOME: name = "Home"; break;
          case SDLK_END: name = "End"; break;
          case SDLK_PAGEUP: name = "PageUp"; break;
          case SDLK_PAGEDOWN: name = "PageDown"; break;
          case SDLK_INSERT: name = "Insert"; break;
          default: break;
        }
      }
      if (name) jsvObjectSetChildAndUnLock(o,"key", jsvNewFromString(name));
      jsiQueueObjectCallbacks(E, sendKeyEvent, &o, 1);
      jsvUnLock2(E,o);
    }
  }
}

void lcdSetCallbacks_SDL(JsGraphics *gfx) {
  gfx->setPixel = lcdSetPixel_SDL;
  gfx->getPixel = lcdGetPixel_SDL;
  gfx->fillRect = lcdFillRect_SDL;
  // FIXME: idle callback would be a great idea to save lock/unlock
}
