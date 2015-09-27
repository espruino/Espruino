/*
 * network_esp8266_board.h
 *
 *  Created on: Aug 29, 2015
 *      Author: kolban
 */

#ifndef LIBS_NETWORK_ESP8266_NETWORK_ESP8266_H_
#define LIBS_NETWORK_ESP8266_NETWORK_ESP8266_H_

#include "network.h"
void netInit_esp8266_board();
void netSetCallbacks_esp8266_board(JsNetwork *net);
void esp8266_dumpSocket(int socketId);
int  net_ESP8266_BOARD_accept(JsNetwork *net, int serverSckt);
int  net_ESP8266_BOARD_recv(JsNetwork *net, int sckt, void *buf, size_t len);
int  net_ESP8266_BOARD_send(JsNetwork *net, int sckt, const void *buf, size_t len);
void net_ESP8266_BOARD_idle(JsNetwork *net);
bool net_ESP8266_BOARD_checkError(JsNetwork *net);
int  net_ESP8266_BOARD_createSocket(JsNetwork *net, uint32_t ipAddress, unsigned short port);
void net_ESP8266_BOARD_closeSocket(JsNetwork *net, int sckt);
void net_ESP8266_BOARD_gethostbyname(JsNetwork *net, char *hostName, uint32_t *outIp);
#endif /* LIBS_NETWORK_ESP8266_NETWORK_ESP8266_H_ */
