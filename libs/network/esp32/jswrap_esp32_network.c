#include "esp_log.h"

#include "network.h"
#include "jswrap_esp32_network.h"

// Tag for ESP-IDF logging
static char *tag = "jswrap_esp32_network";

/**
 * Perform a soft initialization of ESP32 networking.
 */
void jswrap_ESP32_wifi_soft_init() {
	ESP_LOGD(tag, ">> jswrap_ESP32_wifi_soft_init");
	JsNetwork net;
	networkCreate(&net, JSNETWORKTYPE_SOCKET);	
	ESP_LOGD(tag, "<< jswrap_ESP32_wifi_soft_init");	
}
