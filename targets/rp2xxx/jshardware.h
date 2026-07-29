#ifndef JSHARDWARE_RP2_H
#define JSHARDWARE_RP2_H

/* Keep this shim so local includes still work, but use Espruino's real API. */
#include "../../src/jshardware.h"

void rp2EarlyLog(const char *msg);
void rp2EarlyLogf(const char *fmt, ...);
void rp2UsbInitNow(void);

#endif
