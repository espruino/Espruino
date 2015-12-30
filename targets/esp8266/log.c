// Copyright 2015 by Thorsten von Eicken, see LICENSE.txt

#include <user_interface.h>
#include <osapi.h>
#include <mem.h>
#include <uart.h>
#include <espmissingincludes.h>

#define _GCC_WRAP_STDINT_H
typedef long long int64_t;

#include "jsvar.h"
#include "log.h"

#ifdef LOG_DBG
#define DBG(format, ...) os_printf(format, ## __VA_ARGS__)
#else
#define DBG(format, ...) do { } while(0)
#endif

// Whether to prefix lines in memory buffer with a millisecond timestamp: 0=off, 1=on
#define LOG_TS 1

// Log for the esp8266 to replace outputting to uart1.
// The log has a ~1KB circular in-memory buffer which os_printf prints into and
// that can be retrieved line by line

// see console.c for invariants (same here)
//#define BUF_MAX (536*2)
#define BUF_MAX (536/2)
static char *log_buf = 0;               // buffer for log
static int log_wr, log_rd;              // next position to write, next position to read
static uint8_t log_mode = LOG_MODE_OFF; // logging mode
static bool log_newline;                // at start of a new line

// write to the uart designated for logging
static void uart_write_char(char c) {
  if (log_mode == LOG_MODE_ON1)
    uart_tx_one_char(1, c);
  else
    uart_tx_one_char(0, c);
}

#if 0
// called from wifi reset timer to turn UART on when we loose wifi and back off
// when we connect to wifi AP. Here this is gated by the flash setting
void ICACHE_FLASH_ATTR
log_uart(bool enable) {
  if (!enable && !log_no_uart && flashConfig.log_mode < LOG_MODE_ON0) {
    // we're asked to turn uart off, and uart is on, and the flash setting isn't always-on
    DBG("Turning OFF uart log\n");
    os_delay_us(4*1000L); // time for uart to flush
    log_no_uart = !enable;
  } else if (enable && log_no_uart && flashConfig.log_mode != LOG_MODE_OFF) {
    // we're asked to turn uart on, and uart is off, and the flash setting isn't always-off
    log_no_uart = !enable;
    DBG("Turning ON uart log\n");
  }
}
#endif

// write a character into the log buffer without special newline handling
static void log_write(char c) {
  if (!log_buf) return;
  log_buf[log_wr] = c;
  log_wr = (log_wr+1) % BUF_MAX;
  if (log_wr == log_rd) {
    log_rd = (log_rd+1) % BUF_MAX; // full, eat first char
  }
}

// write a character to the log buffer and the uart, and handle newlines specially
static void log_write_char(char c) {
  // log timestamp
  if (log_newline) {
    char buff[16];
    int l = os_sprintf(buff, "%6d> ", (system_get_time()/1000)%1000000);
    if (log_mode > LOG_MODE_MEM)
      for (int i=0; i<l; i++) uart_write_char(buff[i]);
#if LOG_TS
    for (int i=0; i<l; i++) log_write(buff[i]);
#endif
    log_newline = false;
  }
  if (c == '\n') log_newline = true;
  // Uart output unless disabled
  if (log_mode > LOG_MODE_MEM) {
    if (c == '\n') uart_write_char('\r');
    uart_write_char(c);
  }
  // Store in log buffer
  if (c == '\n') log_write('\r');
  log_write(c);
}

void esp8266_logInit(uint8_t mode) {
  if (mode == log_mode) return;
  if (mode == LOG_MODE_OFF) {
    if (log_buf) os_free(log_buf);
    log_buf = NULL;
  } else if (!log_buf) {
    log_buf = os_zalloc(BUF_MAX); // if this fails it's OK, we will simply not log there...
    log_wr = 0;
    log_rd = 0;
  }
  log_mode = mode;
  os_install_putc1((void *)log_write_char);
}

JsVar *esp8266_logGetLine() {
  JsVar *ret;
  char buf[130];
  buf[0] = 0;
  // return empty string if logging is off or we have no buffer
  if (log_mode == LOG_MODE_OFF || !log_buf) {
    return jsvNewFromString(buf); // null string
  }
  // collect a log line
  short i = 0;
  while (log_rd != log_wr && i < 128) {
    char ch = log_buf[log_rd++];
    buf[i++] = ch;
    if (log_rd >= BUF_MAX) log_rd = 0;
    if (ch == '\n') break;
  }
  buf[i++] = 0;
  //for (short j=0; j<i; j++) uart_tx_one_char(1, buf[j]);
  return jsvNewFromString(buf);
}

#if 0
int ICACHE_FLASH_ATTR
ajaxLog(HttpdConnData *connData) {
  char buff[2048];
  int len; // length of text in buff
  int log_len = (log_wr+BUF_MAX-log_rd) % BUF_MAX; // num chars in log_buf
  int start = 0; // offset onto log_wr to start sending out chars

  if (connData->conn==NULL) return HTTPD_CGI_DONE; // Connection aborted. Clean up.
  jsonHeader(connData, 200);

  // figure out where to start in buffer based on URI param
  len = httpdFindArg(connData->getArgs, "start", buff, sizeof(buff));
  if (len > 0) {
    start = atoi(buff);
    if (start < log_pos) {
      start = 0;
    } else if (start >= log_pos+log_len) {
      start = log_len;
    } else {
      start = start - log_pos;
    }
  }

  // start outputting
  len = os_sprintf(buff, "{\"len\":%d, \"start\":%d, \"text\": \"",
      log_len-start, log_pos+start);

  int rd = (log_rd+start) % BUF_MAX;
  while (len < 2040 && rd != log_wr) {
    uint8_t c = log_buf[rd];
    if (c == '\\' || c == '"') {
      buff[len++] = '\\';
      buff[len++] = c;
    } else if (c < ' ') {
      len += os_sprintf(buff+len, "\\u%04x", c);
    } else {
      buff[len++] = c;
    }
    rd = (rd + 1) % BUF_MAX;
  }
  os_strcpy(buff+len, "\"}"); len+=2;
  httpdSend(connData, buff, len);
  return HTTPD_CGI_DONE;
}

static char *dbg_mode[] = { "auto", "off", "on0", "on1" };

int ICACHE_FLASH_ATTR
ajaxLogDbg(HttpdConnData *connData) {
  if (connData->conn==NULL) return HTTPD_CGI_DONE; // Connection aborted. Clean up.
  char buff[512];
  int len, status = 400;
  len = httpdFindArg(connData->getArgs, "mode", buff, sizeof(buff));
  if (len > 0) {
    int8_t mode = -1;
    if (os_strcmp(buff, "auto") == 0)  mode = LOG_MODE_AUTO;
    if (os_strcmp(buff, "off") == 0)   mode = LOG_MODE_OFF;
    if (os_strcmp(buff, "on0") == 0) mode = LOG_MODE_ON0;
    if (os_strcmp(buff, "on1") == 0) mode = LOG_MODE_ON1;
    if (mode >= 0) {
      flashConfig.log_mode = mode;
      if (mode != LOG_MODE_AUTO) log_uart(mode >= LOG_MODE_ON0);
      status = configSave() ? 200 : 400;
    }
  } else if (connData->requestType == HTTPD_METHOD_GET) {
    status = 200;
  }

  jsonHeader(connData, status);
  os_sprintf(buff, "{\"mode\": \"%s\"}", dbg_mode[flashConfig.log_mode]);
  httpdSend(connData, buff, -1);
  return HTTPD_CGI_DONE;
}
#endif

