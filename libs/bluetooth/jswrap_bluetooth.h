#include "jspin.h"

// public static methods.
void jswrap_nrf_bluetooth_init(void);
void jswrap_nrf_bluetooth_send_string(Pin* string_to_send, int length);

void jswrap_nrf_bluetooth_enable_com(void);
void jswrap_nrf_bluetooth_disable_com(void);

void jswrap_nrf_bluetooth_sleep(void); // maybe these should return err_code?
void jswrap_nrf_bluetooth_wake(void);
