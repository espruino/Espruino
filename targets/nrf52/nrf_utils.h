#ifndef NRF_UTILS_H__
#define NRF_UTILS_H__

#include "nrf.h"

#define LFCLK_FREQ = 32768
#define LFCLK_PRESCALER = 0


// Configure the low frequency clock to use the external 32.768 kHz crystal as a source. Start this clock.
void lfclk_config_and_start(void);

// Configure the RTC to default settings (ticks every 1/32768 seconds) and then start it.
void rtc1_config_and_start(void);

#endif // NRF_UTILS_H__
