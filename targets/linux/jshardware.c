/*
 * jshardware.c
 *
 *  Created on: 8 Aug 2012
 *      Author: gw
 */

 #include <stdlib.h>
 #include <string.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <sys/time.h>
 #include <sys/select.h>
 #include <termios.h>
 #include <signal.h>

#include "jshardware.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"

// ----------------------------------------------------------------------------
// for non-blocking IO
struct termios orig_termios;

void reset_terminal_mode()
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int getch()
{
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        if (c=='\3') exit(0); // ctrl-c
        return c;
    }
}

void jshInit() {
  struct termios new_termios;

  /* take two copies - one for now, one for later */
  tcgetattr(0, &orig_termios);
  memcpy(&new_termios, &orig_termios, sizeof(new_termios));

  /* register cleanup handler, and set the new terminal mode */
  atexit(reset_terminal_mode);
  cfmakeraw(&new_termios);
  tcsetattr(0, TCSANOW, &new_termios);
}

void jshKill() {
}

void jshIdle() {
  while (kbhit()) {
    jshPushIOCharEvent(EV_USBSERIAL, (char)getch());
  }
}

// ----------------------------------------------------------------------------

bool jshIsUSBSERIALConnected() {
  return false;
}

JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
  return (JsSysTime)(ms*1000);
}

JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
  return ((JsVarFloat)time)/1000;
}


JsSysTime jshGetSystemTime() {
  struct timeval tm;
  gettimeofday(&tm, 0);
  return tm.tv_sec*1000000L + tm.tv_usec;
}

// ----------------------------------------------------------------------------

int jshGetPinFromString(const char *s) {
  return -1;
}

bool jshPinInput(int pin) {
  bool value = false;
  return value;
}

JsVarFloat jshPinAnalog(int pin) {
  JsVarFloat value = 0;
  return value;
}


void jshPinOutput(int pin, bool value) {
}

void jshPinAnalogOutput(int pin, JsVarFloat value) {
}

void jshPinPulse(int pin, bool value, JsVarFloat time) {
}

void jshPinWatch(int pin, bool shouldWatch) {
}

bool jshIsEventForPin(IOEvent *event, int pin) {
  return false;
}

void jshUSARTSetup(IOEventFlags device, int baudRate) {
}

/** Kick a device into action (if required). For instance we may need
 * to set up interrupts */
void jshUSARTKick(IOEventFlags device) {
}


void jshSaveToFlash() {
  FILE *f = fopen("TinyJSC.state","wb");
  if (f) {
    jsiConsolePrint("\nSaving ");
    jsiConsolePrintInt(jsvGetVarDataSize());
    jsiConsolePrint(" bytes...");
    fwrite(jsvGetVarDataPointer(),1,jsvGetVarDataSize(),f);
    fclose(f);
    jsiConsolePrint("\nDone!\n>");
  } else {
    jsiConsolePrint("\nFile Open Failed... \n>");
  }
}

void jshLoadFromFlash() {
  FILE *f = fopen("TinyJSC.state","rb");
  if (f) {
    jsiConsolePrint("\nLoading ");
    jsiConsolePrintInt(jsvGetVarDataSize());
    jsiConsolePrint(" bytes...\n>");
    fread(jsvGetVarDataPointer(),1,jsvGetVarDataSize(),f);
    fclose(f);
  } else {
    jsiConsolePrint("\nFile Open Failed... \n>");
  }
}

bool jshFlashContainsCode() {
  FILE *f = fopen("TinyJSC.state","rb");
  if (f) fclose(f);
  return f!=0;
}

/// Enter simple sleep mode (can be woken up by interrupts)
void jshSleep() {
  usleep(10*1000);
}

