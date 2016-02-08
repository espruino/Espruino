#include "jspin.h"

// public static methods.
void jswrap_nrf_bluetooth_init(void);

void jswrap_nrf_bluetooth_sleep(void); // maybe these should return err_code?
void jswrap_nrf_bluetooth_wake(void);

JsVarFloat jswrap_nrf_bluetooth_getBattery(void);
void jswrap_nrf_bluetooth_setAdvertising(JsVar *data);
void jswrap_nrf_bluetooth_setScan(JsVar *callback);

bool jswrap_nrf_idle();
