#include "jsutils.h"
#include "jsvar.h"
#include "jsparse.h"
#include "jsinteractive.h"

void jswrap_nrf_bluetooth_init(void);
void jswrap_nrf_bluetooth_send_string(Pin* string_to_send, int length);
