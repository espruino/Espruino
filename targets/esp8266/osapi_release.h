#include <osapi.h>

#ifdef RELEASE
#undef os_printf
#define os_printf(format, ...) do { } while(0)
#endif
