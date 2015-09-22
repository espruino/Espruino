#ifndef TEMPERATURE_H__
#define TEMPERATURE_H__

void jswrap_nrf_temper_init();
int jswrap_nrf_temper_read();
void jswrap_nrf_busy_blink(int numberOfLoops);
int jswrap_nrf_get_random_number();


#endif // TEMPERATURE_H__
