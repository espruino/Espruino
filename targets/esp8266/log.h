#ifndef LOG_H
#define LOG_H

#define LOG_MODE_OFF  0  // completely off
#define LOG_MODE_MEM  1  // in-memory only
#define LOG_MODE_ON0  2  // in-memory and uart0
#define LOG_MODE_ON1  3  // in-memory and uart1

void esp8266_logInit(uint8_t mode);
JsVar *esp8266_logGetLine();

#if 0
int ajaxLog(HttpdConnData *connData);
int ajaxLogDbg(HttpdConnData *connData);
#endif

#endif
