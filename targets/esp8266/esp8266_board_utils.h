/*
 * esp8266_board_utils.h
 *
 *  Created on: Sep 3, 2015
 *      Author: kolban
 */

#ifndef TARGETS_ESP8266_ESP8266_BOARD_UTILS_H_
#define TARGETS_ESP8266_ESP8266_BOARD_UTILS_H_

// Return a string representation of an ESP8266 error.
const char *esp8266_errorToString(sint8 err);
void esp8266_board_writeString(uint8 *buffer, size_t length);

#endif /* TARGETS_ESP8266_ESP8266_BOARD_UTILS_H_ */
