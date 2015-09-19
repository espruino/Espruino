#include "jswrap_temperature.h"
#include "jsinteractive.h"

#include "nrf.h"
#include "nrf_temp.h"
#include "nrf_gpio.h"

/*JSON{
	"type": "class",
	"class" : "Temperature"
}*/

/*JSON{
	"type" : "staticmethod",
	"class" : "Temperature",
	"name" : "init", 
	"generate" : "jswrap_nrf_temper_init"
}*/
void jswrap_nrf_temper_init() {
	jsiConsolePrint("Initializing the temperature...\r\n");
	nrf_temp_init();
}

/*JSON{
	"type" : "staticmethod",
	"class" : "Temperature",
	"name" : "read", 
	"generate" : "jswrap_nrf_temper_read",
	"return" : ["int","The internal temperature of the nRF52 device [C]."]
}*/
int jswrap_nrf_temper_read() {

	jsiConsolePrint("Reading the temperature...\r\n");

	int32_t volatile nrf_temp;

	NRF_TEMP->TASKS_START = 1;
	while (NRF_TEMP->EVENTS_DATARDY == 0)
	{
		// Do nothing...
	}
	NRF_TEMP->EVENTS_DATARDY = 0;

	nrf_temp = (nrf_temp_read() / 4);

	NRF_TEMP->TASKS_STOP = 1;

	return (int) nrf_temp;
}

/*JSON{
	"type" : "staticmethod",
	"class" : "Temperature",
	"name" : "rand", 
	"generate" : "jswrap_nrf_get_random_number",
	"return" : ["int","The random number generated."]
}*/
int jswrap_nrf_get_random_number() {

  jsiConsolePrint("Generating a random number from internal noise...\r\n");

  NRF_RNG->CONFIG = 0x00000001; // Use the bias generator.
  NRF_RNG->TASKS_START = 1;

  while (NRF_RNG->EVENTS_VALRDY != 1)
  {
    // Do nothing.
  }

  NRF_RNG -> EVENTS_VALRDY = 0;

  uint8_t rand_num = (uint8_t) NRF_RNG->VALUE;

  NRF_RNG -> TASKS_STOP = 1;

  return (int) rand_num;

}

/*JSON{
	"type" : "staticmethod",
	"class" : "Temperature",
	"name" : "blink", 
	"generate" : "jswrap_nrf_busy_blink",
	"params" : [
    ["numberOfLopps","int","The number of loops to wait for between blinking the LED."]
  ]
}*/
void jswrap_nrf_busy_blink(int numberOfLoops) {

  nrf_gpio_cfg_output(17);
  nrf_gpio_pin_set(17);

  int i;
  for (i = 0; i < numberOfLoops; i++)
  {
  	// do nothing...
  }

  nrf_gpio_pin_clear(17);

  for (i = 0; i < numberOfLoops; i++)
  {
  	// do nothing...
  }

  nrf_gpio_pin_set(17);

}
