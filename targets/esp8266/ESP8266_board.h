/*
 * ESP8266_board.h
 *
 *  Created on: Aug 25, 2015
 *      Author: kolban
 */

#ifndef TARGETS_ESP8266_ESP8266_BOARD_H_
#define TARGETS_ESP8266_ESP8266_BOARD_H_
#include <user_interface.h>
// Define the task ids for the APP event handler
#define TASK_APP_MAINLOOP ((os_signal_t)1)
#define TASK_APP_RX_DATA ((os_signal_t)2)
#define TASK_APP_QUEUE USER_TASK_PRIO_1


#endif /* TARGETS_ESP8266_ESP8266_BOARD_H_ */
