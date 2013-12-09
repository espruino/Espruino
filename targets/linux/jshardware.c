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
 * Platform Specific part of Hardware interface Layer
 * ----------------------------------------------------------------------------
 */
 #include <stdlib.h>
 #include <string.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <sys/time.h>
 #include <sys/select.h>
 #include <termios.h>
 #include <signal.h>
 #include <inttypes.h>

#include "jshardware.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"


#ifdef CARAMBOLA
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define SYSFS_GPIO_MIN 1
#define SYSFS_GPIO_MAX 14
#endif
#ifdef RASPBERRYPI
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define SYSFS_GPIO_MIN 0
#define SYSFS_GPIO_MAX 27
#endif

// ----------------------------------------------------------------------------
#ifdef SYSFS_GPIO_DIR

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

bool gpioShouldWatch[SYSFS_GPIO_MAX+1]; // whether we should watch this pin for changes
bool gpioLastState[SYSFS_GPIO_MAX+1]; // the last state of this pin
JshPinState gpioState[SYSFS_GPIO_MAX+1]; // will be set to UNDEFINED if it isn't exported
IOEventFlags gpioEventFlags[SYSFS_GPIO_MAX+1];

// functions for accessing the sysfs GPIO
void sysfs_write(const char *path, const char *data) {
/*  jsiConsolePrint(path);
  jsiConsolePrint(" = '");
  jsiConsolePrint(data);
  jsiConsolePrint("'\n");*/
  int f = open(path, O_WRONLY);
  if (f>=0) {
    write(f, data, strlen(data));
    close(f);
  } 
}

void sysfs_write_int(const char *path, JsVarInt val) {
  char buf[20];
  itoa(val, buf, 10);
  sysfs_write(path, buf);
}

void sysfs_read(const char *path, char *data, unsigned int len) {
  int amt = 0;
  int f = open(path, O_RDONLY);
  if (f>=0) {
    amt = read(f, data, len-1);
    close(f);
  } 
  if (amt<0) amt=0;
  data[amt]=0;
}

JsVarInt sysfs_read_int(const char *path) {
  char buf[20];
  sysfs_read(path, buf, sizeof(buf));
  return stringToIntWithRadix(buf, 10);
}

// ----------------------------------------------------------------------------

IOEventFlags pinToEVEXTI(Pin pin) {
  return gpioEventFlags[pin];
}

IOEventFlags getNewEVEXTI() {
  int i;
  for (i=0;i<16;i++) {
    IOEventFlags evFlag = (IOEventFlags)(EV_EXTI0+i);
    Pin pin;
    bool found = false;
    for (pin=SYSFS_GPIO_MIN;pin<=SYSFS_GPIO_MAX;pin++) 
      if (gpioEventFlags[pin] == evFlag)
        found = true;
    if (!found)
      return evFlag;
  }
  return 0;
}
#else
IOEventFlags pinToEVEXTI(Pin pin) {
  return 0;
}

#endif

// ----------------------------------------------------------------------------
// for non-blocking IO
struct termios orig_termios;
static int terminal_set = 0;

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
  if (!terminal_set) {
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
    terminal_set = 1;
  }

#ifdef SYSFS_GPIO_DIR
  int i;
  for (i=0;i<=SYSFS_GPIO_MAX;i++) {
    gpioState[i] = JSHPINSTATE_UNDEFINED;
    gpioShouldWatch[i] = false;
    gpioEventFlags[i] = 0;
  }
#endif
}

void jshKill() {
#ifdef SYSFS_GPIO_DIR
  int i;
  // unexport any GPIO that we exported
  for (i=0;i<=SYSFS_GPIO_MAX;i++)
    if (gpioState[i] != JSHPINSTATE_UNDEFINED)
      sysfs_write_int(SYSFS_GPIO_DIR"/unexport", i);
#endif
}

void jshIdle() {
  while (kbhit()) {
    jshPushIOCharEvent(EV_USBSERIAL, (char)getch());
  }

#ifdef SYSFS_GPIO_DIR
  Pin pin;
  for (pin=SYSFS_GPIO_MIN;pin<=SYSFS_GPIO_MAX;pin++)
    if (gpioShouldWatch[pin]) {
      bool state = jshPinGetValue(pin);
      if (state != gpioLastState[pin]) {
        jshPushIOEvent(pinToEVEXTI(pin) | (state?EV_EXTI_IS_HIGH:0), jshGetSystemTime());
        gpioLastState[pin] = state;
      }
    }
#endif
}

// ----------------------------------------------------------------------------

int jshGetSerialNumber(unsigned char *data, int maxChars) {
  long initialSerial = 0;
  long long serial = 0xDEADDEADDEADDEADL; 
  FILE *f = fopen("/proc/cpuinfo", "r");
  if (f) {
    char line[256]; 
    while (fgets(line, 256, f)) {
      if (strncmp(line, "Serial", 6) == 0) {
        char serial_string[16 + 1];
        strcpy(serial_string, strchr(line, ':') + 2);
        serial = stringToIntWithRadix(serial_string, 16);
      }
    }
    fclose(f);
  }
  memcpy(&data[0], &initialSerial, 4);
  memcpy(&data[4], &serial, 8);
  return 12;
}

unsigned int jshGetRegistrationCode() {
  unsigned int code = 0xFFFFFFFF;
  FILE *f = fopen("espruino.code","rb");
  if (f) {
    fread(&code,1,4,f);
    fclose(f);
  }
  return code;
}

void jshSetRegistrationCode(unsigned int code) {
  FILE *f = fopen("espruino.code","wb");
  if (f) {
    fwrite(&code,1,4,f);
    fclose(f);
  }
}

// ----------------------------------------------------------------------------

void jshInterruptOff() {
}

void jshInterruptOn() {
}

void jshDelayMicroseconds(int microsec) {
  usleep(microsec);
}

bool jshGetPinStateIsManual(Pin pin) { 
  return false; 
}

void jshSetPinStateIsManual(Pin pin, bool manual) { 
}

void jshPinSetState(Pin pin, JshPinState state) {
#ifdef SYSFS_GPIO_DIR
  if (gpioState[pin] != state) {
    if (gpioState[pin] == JSHPINSTATE_UNDEFINED)
      sysfs_write_int(SYSFS_GPIO_DIR"/export", pin);
    char path[64] = SYSFS_GPIO_DIR"/gpio";
    itoa(pin, &path[strlen(path)], 10);
    strcat(&path[strlen(path)], "/direction");
    sysfs_write(path, JSHPINSTATE_IS_OUTPUT(state)?"out":"in");
    gpioState[pin] = state;
  }
#endif
}

JshPinState jshPinGetState(Pin pin) {
#ifdef SYSFS_GPIO_DIR
  return gpioState[pin];
#endif
  return JSHPINSTATE_UNDEFINED;
}

void jshPinSetValue(Pin pin, bool value) {
#ifdef SYSFS_GPIO_DIR
  char path[64] = SYSFS_GPIO_DIR"/gpio";
  itoa(pin, &path[strlen(path)], 10);
  strcat(&path[strlen(path)], "/value");
  sysfs_write_int(path, value?1:0);
#endif
}

bool jshPinGetValue(Pin pin) {
#ifdef SYSFS_GPIO_DIR
  char path[64] = SYSFS_GPIO_DIR"/gpio";
  itoa(pin, &path[strlen(path)], 10);
  strcat(&path[strlen(path)], "/value");
  return sysfs_read_int(path);
#else
  return false;
#endif
}

bool jshIsPinValid(Pin pin) {
#ifdef SYSFS_GPIO_DIR
  return pin>=SYSFS_GPIO_MIN && pin<=SYSFS_GPIO_MAX;
#else
  return true;
#endif
}

bool jshIsDeviceInitialised(IOEventFlags device) { return true; }

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

Pin jshGetPinFromString(const char *s) {
#ifdef SYSFS_GPIO_DIR
  if ((s[0]=='D') && s[1]) { // first 6 are analogs
    Pin pin = 127;
    if (!s[2] && (s[1]>='0' && s[1]<='9')) { // D0-D9
      pin = (Pin)(s[1]-'0');
    } else if (!s[3] && (s[1]>='1' && s[1]<='3' && s[2]>='0' && s[2]<='9')) { // D1X-D3X
      pin = (Pin)((s[1]-'0')*10 + (s[2]-'0'));
    }
    if (pin>=SYSFS_GPIO_MIN && pin<=SYSFS_GPIO_MAX)
      return pin;
  }
#endif
#ifndef CARAMBOLA
  if (!strcmp(s,"D0")) return (Pin)0;
  if (!strcmp(s,"D1")) return (Pin)1;
  if (!strcmp(s,"D2")) return (Pin)2;
  if (!strcmp(s,"D3")) return (Pin)3;
  if (!strcmp(s,"LED1")) return (Pin)1; 
  if (!strcmp(s,"LED2")) return (Pin)2; 
  if (!strcmp(s,"LED3")) return (Pin)3; 
  if (!strcmp(s,"LED4")) return (Pin)4; 
  if (!strcmp(s,"BTN")) return (Pin)5; 
#endif
  return -1;
}

/** Write the pin name to a string. String must have at least 8 characters (to be safe) */
void jshGetPinString(char *result, Pin pin) {
  result[0]='D';
  itoa(pin,&result[1],10);
}

bool jshPinInput(Pin pin) {
  bool value = false;
  if (jshIsPinValid(pin)) {
    jshPinSetState(pin, JSHPINSTATE_GPIO_IN);

    value = jshPinGetValue(pin);
  } else jsError("Invalid pin!");
  return value;
}

JsVarFloat jshPinAnalog(Pin pin) {
  JsVarFloat value = 0;
  jsError("Analog is not supported on this device.");
  return value;
}


void jshPinOutput(Pin pin, bool value) {
  if (jshIsPinValid(pin)) {
    jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
    jshPinSetValue(pin, value);
  } else jsError("Invalid pin!");
}

bool jshPinOutputAtTime(JsSysTime time, Pin pin, bool value) {
 // FIXME
}

void jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq) { // if freq<=0, the default is used
}

void jshPinPulse(Pin pin, bool value, JsVarFloat time) {
  if (jshIsPinValid(pin)) {
    jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
    jshPinSetValue(pin, value);
    usleep(time*1000000);
    jshPinSetValue(pin, !value);
  } else jsError("Invalid pin!");
}

void jshPinWatch(Pin pin, bool shouldWatch) {
  if (jshIsPinValid(pin)) {
#ifdef SYSFS_GPIO_DIR
    IOEventFlags exti = getNewEVEXTI();
    if (shouldWatch) {
      if (exti) {
        gpioShouldWatch[pin] = true;
        gpioEventFlags[pin] = exti;
        jshPinSetState(pin, JSHPINSTATE_GPIO_IN);
        gpioLastState[pin] = jshPinGetValue(pin);
      } else 
        jsError("You can only have a maximum of 16 watches!");
    }
    if (!shouldWatch || !exti) {
      gpioShouldWatch[pin] = false;
      gpioEventFlags[pin] = 0;
    }
#endif
  } else jsError("Invalid pin!");
}

bool jshGetWatchedPinState(IOEventFlags device) {
#ifdef SYSFS_GPIO_DIR
  Pin i;
  for (i=0;i<=SYSFS_GPIO_MAX;i++)
    if (gpioEventFlags[i]==device)
      return jshPinGetValue(i);
#endif
  return false;
}

bool jshIsEventForPin(IOEvent *event, Pin pin) {
  return IOEVENTFLAGS_GETTYPE(event->flags) == pinToEVEXTI(pin);
}

void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {
}

/** Kick a device into action (if required). For instance we may need
 * to set up interrupts */
void jshUSARTKick(IOEventFlags device) {
}

void jshSPISetup(IOEventFlags device, JshSPIInfo *inf) {
}

/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(IOEventFlags device, int data) {
}

/** Send 16 bit data through the given SPI device. */
void jshSPISend16(IOEventFlags device, int data) {
  jshSPISend(device, data>>8);
  jshSPISend(device, data&255);
}

/** Set whether to send 16 bits or 8 over SPI */
void jshSPISet16(IOEventFlags device, bool is16) {
}

void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf) {
}

void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data) {
}

void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes, unsigned char *data) {
}


void jshSaveToFlash() {
  FILE *f = fopen("espruino.state","wb");
  if (f) {
    unsigned int jsVarCount = jsvGetMemoryTotal();
    jsiConsolePrintf("\nSaving %d bytes...", jsVarCount*sizeof(JsVar));
    JsVarRef i;

    for (i=1;i<=jsVarCount;i++) {
      fwrite(_jsvGetAddressOf(i),1,sizeof(JsVar),f);
    }
    fclose(f);
    jsiConsolePrint("\nDone!\n>");
  } else {
    jsiConsolePrint("\nFile Open Failed... \n>");
  }
}

void jshLoadFromFlash() {
  FILE *f = fopen("espruino.state","rb");
  if (f) {
    fseek(f, 0L, SEEK_END);
    unsigned int fileSize = ftell(f);
    fseek(f, 0L, SEEK_SET);

    jsiConsolePrintf("\nLoading %d bytes...\n>", fileSize);

    unsigned int jsVarCount = fileSize / sizeof(JsVar);
    jsvSetMemoryTotal(jsVarCount);
    JsVarRef i;
    for (i=1;i<=jsVarCount;i++) {
      fread(_jsvGetAddressOf(i),1,sizeof(JsVar),f);
    }
    fclose(f);
  } else {
    jsiConsolePrint("\nFile Open Failed... \n>");
  }
}

bool jshFlashContainsCode() {
  FILE *f = fopen("espruino.state","rb");
  if (f) fclose(f);
  return f!=0;
}

/// Enter simple sleep mode (can be woken up by interrupts)
void jshSleep() {
  bool hasWatches = false;
#ifdef SYSFS_GPIO_DIR
  Pin pin;
  for (pin=SYSFS_GPIO_MIN;pin<=SYSFS_GPIO_MAX;pin++)
    if (gpioShouldWatch[pin]) hasWatches = true;
#endif

  usleep(hasWatches ? 1000 : (10*1000)); // don't sleep much if we have watches - we need to keep polling them
}

void jshBitBang(Pin pin, JsVarFloat t0h, JsVarFloat t0l, JsVarFloat t1h, JsVarFloat t1l, JsVar *str) {
  jsError("Bit banging not implemented on Linux");
}
