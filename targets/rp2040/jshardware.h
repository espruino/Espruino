#ifndef JSHARDWARE_RP2040_H
#define JSHARDWARE_RP2040_H

/* Keep this shim so local includes still work, but use Espruino's real API. */
#include "../../src/jshardware.h"

void rp2040EarlyLog(const char *msg);
void rp2040EarlyLogf(const char *fmt, ...);
void rp2040UsbInitNow(void);

#endif
