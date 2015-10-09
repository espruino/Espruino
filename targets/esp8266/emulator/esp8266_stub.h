/*
 * esp8266_stub.h
 *
 *  Created on: Oct 4, 2015
 *      Author: kolban
 */

#ifndef ESP8266_STUB_H_
#define ESP8266_STUB_H_

#include <c_types.h>
#include <ip_addr.h>
#include <espconn.h>
struct stub_ESP8266Socket {
  struct espconn *pEspconn;
  int socketId;
};



#endif /* ESP8266_STUB_H_ */
