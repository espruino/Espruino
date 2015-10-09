/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains ESP8266 board specific functions.
 * ----------------------------------------------------------------------------
 */


/**
 * This source file contains a "shim" or emulation of the ESP8266 functions for a
 * Windows or Linux platform.  The goal is to be able to compile and run C based
 * applications on Windows or Linux that emulate what a real ESP8266 device would
 * do.
 *
 * Windows and Linux specific APIs are broken out into separate files.  This is
 * a common file for both Windows and Linux.
 *
 * During compilation, it is assumed that the actual ESP8266 SDK header files are
 * available.
 *
 */
#include <c_types.h>
#include <user_interface.h>
#include <espconn.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <stdlib.h>

#include <sys/time.h>

#include "esp8266_stub.h"
#include "esp8266_stub_sockets.h"

/**
 * \brief A holder for the ESP8266 initialization done callback.
 */
static init_done_cb_t g_initDoneCB;

/**
 * \brief A holder for the ESP8266 WiFi event callback.
 */
static wifi_event_handler_cb_t g_wifiEventHandlerCB;

/**
 * \brief A holder for the ESP8266 WiFi operation mode.
 */
static uint8 g_wifiCurrentOpMode;

/**
 * \brief The current WiFi connection status.
 */
static uint8_t g_wifiStationConnectStatus;

#define MAX_OPEN_SOCKETS (10)


struct stub_ESP8266Socket g_esp8266Sockets[MAX_OPEN_SOCKETS];

/**
 * The ESP8266 has the concept of tasks that can be executed at a later time.
 * This data structure describes the content of such as task as known to the
 * emulation shim.
 */
struct stub_taskData {
  os_event_t *queue;     //!< The queue that holds the tasks to be executed.
  uint8 maxLen;          //!< The maximum length of the queue that holds the tasks.
  uint8 count;           //!< The number of tasks in the queue.
  os_task_t taskHandler; //!< The function that processes ALL tasks for this queue.
  int head;              //!< The head of the queue (where a new entry will be added).
  int tail;              //!< The tail of the queue( where the next entry to be pulled will be taken from).
};


/**
 * The three predefined priorities.
 */
struct stub_taskData prio0TaskData;
struct stub_taskData prio1TaskData;
struct stub_taskData prio2TaskData;

extern void user_init();

/**
 * \brief Initialize the stub.
 * When the stub starts, one time initialization needs to be performed.  This function
 * must be called as early as possible in the stub startup.
 */
static void stubInit() {
  // Set the global values to defaults.
  g_initDoneCB = NULL;
  g_wifiEventHandlerCB = NULL;
  g_wifiStationConnectStatus = STATION_IDLE;

  int i;
  struct stub_ESP8266Socket *pCurrentSocket = g_esp8266Sockets;
  for (i=0; i<MAX_OPEN_SOCKETS; i++) {
    pCurrentSocket->socketId = -1;
    pCurrentSocket->pEspconn = NULL;
  }

  // Initialize the sockets interface.
  esp8266_stub_initSockets();

  // Start the built in telnet server.
  esp8266_stub_startTelnetServer();
}

/**
 * \brief Find the next unused/free socket structure.
 * \return A pointer to a free socket structure or NULL if there are none left.
 */
static struct stub_ESP8266Socket *getNextFreeSocket() {
  int i;
  struct stub_ESP8266Socket *pCurrentSocket = g_esp8266Sockets;
  for (i=0; i<MAX_OPEN_SOCKETS; i++) {
    if (pCurrentSocket->pEspconn == NULL || pCurrentSocket->pEspconn->type == ESPCONN_INVALID) {
      return pCurrentSocket;
    }
  }
  return NULL;
}

/**
 * \brief Release a socket so that it can be reused.
 */
static void releaseSocket(struct stub_ESP8266Socket *pSocket) {
  pSocket->socketId      = -1;
  pSocket->pEspconn->type  = ESPCONN_INVALID;
}

/**
 * \brief Find a socket by Espconn structure.
 */
static struct stub_ESP8266Socket *findByEspconn(struct espconn *pEspconn) {
  int i;
  struct stub_ESP8266Socket *pCurrentSocket = g_esp8266Sockets;
  for (i=0; i<MAX_OPEN_SOCKETS; i++) {
    if (pCurrentSocket->pEspconn == pEspconn) {
      return pCurrentSocket;
    }
  }
  return NULL;
}


/**
 * \brief Execute all tasks on the specific task queue.
 */
static void processSingleTaskQueue(struct stub_taskData *pTaskData) {
  // While there are still tasks on the queue, take each task and call the
  // task handler for that task passing in the task data.

  // We have the possibility where processing a task on the task queue results
  // in a new task on the task queue.  This would mean that we would process a
  // task but the count of total tasks would remain the same ... and we would
  // loop for ever.  To prevent this, we remember where the tail of the queue
  // started and if we look back to where we began, then we give up processing
  // tasks for this go around.
  int startTail = pTaskData->tail;
  while(pTaskData->head != pTaskData->tail) {
    os_event_t *pEvent = &pTaskData->queue[pTaskData->tail];
    pTaskData->taskHandler(pEvent);
    pTaskData->tail = (pTaskData->tail + 1) % pTaskData->maxLen;
    if (pTaskData->tail == startTail) {
      break;
    }
  }
}

/**
 * \brief Process all the tasks for each of the ESP8266 defined task queues.
 */
static void processTaskQueues() {
  processSingleTaskQueue(&prio0TaskData);
  processSingleTaskQueue(&prio1TaskData);
  processSingleTaskQueue(&prio2TaskData);
}

/**
 * \brief Handle data available on the socket.
 * Check if there is data available on the socket and, if there is, handle it.
 */
static void checkForRecvData(struct stub_ESP8266Socket *pSocket) {
  /// Get the data from the pSocket
  uint8_t buf[100];
  int rc;
  rc =  esp8266_stub_recv(pSocket, buf, sizeof(buf));
  // If an error is detected, call the reconnect_callback (assuming one exists).
  if (rc < 0) {
    if (pSocket->pEspconn->proto.tcp->reconnect_callback != NULL) {
      pSocket->pEspconn->proto.tcp->reconnect_callback(pSocket->pEspconn, rc);
    }
  } else if (rc > 0) {
    if (pSocket->pEspconn->recv_callback != NULL) {
      pSocket->pEspconn->recv_callback(pSocket->pEspconn, (char *)buf, rc);
    }
  }
}


/**
 * \brief Check each of the sockets and see if any of them have data for receipt.
 * Process incoming data for each of the open sockets.
 */
static void checkAllForRecvData() {
  int i;
  struct stub_ESP8266Socket *pCurrentSocket = g_esp8266Sockets;
  for (i=0; i<MAX_OPEN_SOCKETS; i++) {
    if (pCurrentSocket->pEspconn != NULL) {
      checkForRecvData(pCurrentSocket);
    }
  }
}


/**
 * \brief Emulate ESP8266 internals processing.
 */
static void esp8266_internalsProcessing() {
  // Examine all the open sockets to see if there is data to be received.
  checkAllForRecvData();
  // Process the task queues.
  processTaskQueues();
}

int main(int argc, char *argv[]) {
  printf("ESP8266 Windows Stub starting ...\n");

  // Set the I/O to be non buffering.
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  stubInit();

  // Invoke the user supplied entry point.
  user_init();

  // If there is a post initialization function, call it now.
  if (g_initDoneCB != NULL) {
    g_initDoneCB();
  }

  // Loop performing internals.
  while(1==1) {
    esp8266_internalsProcessing();
  }

  // Fix ...
  // This works great for the Espruino build, but we need something more generic
  // for generic ESP8266 applications.
  return 0;
}


void system_set_os_print(uint8 onoff) {
  // There is nothing obvious that needs done to support this function at this
  // time.
  //printf(">> system_set_os_print()\n");
}

void os_printf_plus(char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  //printf("os_printf ...\n");
}

void ets_strncpy(char *dest, char *src, size_t num) {
  //printf("ets_strncpy\n");
  strncpy(dest, src, num);
}

void ets_strcpy(char *dest, char *src) {
  //printf("ets_strcpy\n");
  strcpy(dest, src);
}

uint32 system_get_time() {
  //printf("system_get_time\n");
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000 + tv.tv_usec;
}

//FIX
//UartDevice UartDev;

bool wifi_set_opmode_current(uint8 opmode) {
  //printf("ESPSTUB: wifi_set_opmode_current: %s: %d\n",__FILE__, __LINE__);
  g_wifiCurrentOpMode = opmode;
  return true;
}


bool wifi_station_set_config(struct station_config *config) {
  //printf("ESPSTUB: wifi_station_set_config: %s: %d\n", __FILE__, __LINE__);
  return true;
}

void wifi_set_event_handler_cb(wifi_event_handler_cb_t cb)
{
  // Record the event handler for WiFi events in a global variable.
  //printf("ESPSTUB: wifi_set_event_handler_cb: %s: %d\n",__FILE__, __LINE__);
  g_wifiEventHandlerCB = cb;
}

bool wifi_station_connect(void) {
  //printf("ESPSTUB: wifi_station_connect: %s: %d\n",__FILE__, __LINE__);
  if (g_wifiEventHandlerCB != NULL) {
    System_Event_t event;

    event.event = EVENT_STAMODE_CONNECTED;
    g_wifiStationConnectStatus = STATION_CONNECTING;
    g_wifiEventHandlerCB(&event);

    event.event = EVENT_STAMODE_GOT_IP;
    g_wifiStationConnectStatus = STATION_GOT_IP;
    g_wifiEventHandlerCB(&event);
  }
  return true;
}


// TODO: Needs implemented - wifi_softap_set_config_current
bool wifi_softap_set_config_current(struct softap_config *config) {
  printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
  return true;
}

// TODO: Needs implemented - wifi_station_scan
bool wifi_station_scan(struct scan_config *config, scan_done_cb_t cb) {
  printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
  return true;
}

// TODO: Needs implemented - wifi_station_set_auto_connect
bool wifi_station_set_auto_connect(uint8 set) {
  printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
  return true;
}

// TODO: Needs implemented - wifi_station_get_auto_connect
uint8 wifi_station_get_auto_connect(void) {
  printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
  return 0;
}


bool system_os_post(uint8 prio, os_signal_t sig, os_param_t par) {
  // Add a task onto the appropriate task queue.
  //printf("ESPSTUB: system_os_post: %s: %d\n",__FILE__, __LINE__);
  assert(prio == USER_TASK_PRIO_0 || prio == USER_TASK_PRIO_1 || prio == USER_TASK_PRIO_2);
  struct stub_taskData *pTaskData;
  switch(prio) {
    case USER_TASK_PRIO_0:
      pTaskData = &prio0TaskData;
      break;
    case USER_TASK_PRIO_1:
      pTaskData = &prio1TaskData;
      break;
    case USER_TASK_PRIO_2:
      pTaskData = &prio2TaskData;
      break;
  }
  os_event_t *pEvent = &pTaskData->queue[pTaskData->head];
  pEvent->par = par;
  pEvent->sig = sig;
  pTaskData->count++;
  pTaskData->head = (pTaskData->head + 1) % pTaskData->maxLen;
  return true;
}


bool system_os_task(
    os_task_t task,    //!< The callback function to invoke when a task is ready
    uint8 prio,        //!< The priority of the task
    os_event_t *queue, //!< The queue for this priority of tasks
    uint8 qlen         //!< The size of the queue for this priority
  ) {
  assert(prio == USER_TASK_PRIO_0 || prio == USER_TASK_PRIO_1 || prio == USER_TASK_PRIO_2);
  struct stub_taskData *pTaskData;
  switch(prio) {
  case USER_TASK_PRIO_0:
    pTaskData = &prio0TaskData;
    break;
  case USER_TASK_PRIO_1:
    pTaskData = &prio1TaskData;
    break;
  case USER_TASK_PRIO_2:
    pTaskData = &prio2TaskData;
    break;
  }
  pTaskData->taskHandler = task;
  pTaskData->maxLen = qlen;
  pTaskData->queue = queue;
  pTaskData->count = 0;
  pTaskData->head = 0;
  pTaskData->tail = 0;
  //printf("ESPSTUB: system_os_task: %s: %d\n",__FILE__, __LINE__);
  return true;
}

void system_init_done_cb(init_done_cb_t cb) {
  //printf("ESPSTUB: system_init_done_cb: %s: %d\n",__FILE__, __LINE__);
  g_initDoneCB = cb;
}

// TODO: Needs implemented - system_adc_read
uint16 system_adc_read(void){
  printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
  return 0;
}

static struct rst_info rstInfo;
struct rst_info* system_get_rst_info(void) {
  //printf("ESPSTUB: system_get_rst_info: %s: %d\n",__FILE__, __LINE__);
  return &rstInfo;
}

const char *system_get_sdk_version(void){
  //printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
  return "Emulated ESP8266";
}

uint8 system_get_cpu_freq(void) {
  //printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
  return 80;
}

// TODO: Needs implemented - system_get_free_heap_size
uint32 system_get_free_heap_size(void) {
  printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
  return 0;
}

uint8 espconn_tcp_get_max_con(void) {
  //printf("ESPSTUB: espconn_tcp_get_max_con: %s: %d\n",__FILE__, __LINE__);
  return 5;
}

bool wifi_get_ip_info(uint8 if_index, struct ip_info *info) {
  //printf("ESPSTUB: wifi_get_ip_info: %s: %d\n",__FILE__, __LINE__);
  info->ip.addr = esp8266_stub_getLocalIP();
  return true;
}

// TODO: Needs implemented - wifi_station_get_config
bool wifi_station_get_config(struct station_config *config){
  printf("ESPSTUB: wifi_station_get_config: %s: %d\n",__FILE__, __LINE__);
  return true;
}

// TODO: Needs implemented - wifi_softap_get_station_num
uint8 wifi_softap_get_station_num(void){
  printf("ESPSTUB: wifi_softap_get_station_num: %s: %d\n",__FILE__, __LINE__);
  return 0;
}

// TODO: Needs implemented - wifi_softap_get_station_info
struct station_info *wifi_softap_get_station_info(void) {
  printf("ESPSTUB: wifi_softap_get_station_info: %s: %d\n",__FILE__, __LINE__);
  return NULL;
}

// TODO: Needs implemented - gpio_output_set
void gpio_output_set() {
  printf("ESPSTUB: gpio_output_set: %s: %d\n",__FILE__, __LINE__);

}

// TODO: Needs implemented - system_restart
void system_restart() {
  printf("ESPSTUB: system_restart: %s: %d\n",__FILE__, __LINE__);
}

// TODO: Needs implemented - uart_div_modify
void uart_div_modify() {
  printf("ESPSTUB: uart_div_modify: %s: %d\n",__FILE__, __LINE__);
}

void vPortFree(void *ptr) {
  //printf("ESPSTUB: vPortFree: %s: %d\n",__FILE__, __LINE__);
  free(ptr);
}

void* pvPortMalloc(size_t size) {
  //printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
  return malloc(size);
}

// TODO: Needs implemented - ets_install_putc1
void ets_install_putc1() {
  printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
}

// TODO: Needs implemented - ets_isr_unmask
void ets_isr_unmask() {
  printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
}

// TODO: Needs implemented - ets_isr_attach
void ets_isr_attach() {
  printf("ESPSTUB: ets_isr_attach: %s: %d\n",__FILE__, __LINE__);
}

// TODO: Needs implemented - ets_delay_us
void ets_delay_us() {
  printf("ESPSTUB: ets_delay_us: %s: %d\n",__FILE__, __LINE__);
}

void *ets_memcpy(void *dest, void *src, size_t n) {
  //printf("ESPSTUB: ets_memcpy: %s: %d\n",__FILE__, __LINE__);
  memcpy(dest, src, n);
  return dest;
}

void *ets_memset(void *ptr, int value, size_t num) {
  //printf("ESPSTUB: ets_memset: %s: %d\n",__FILE__, __LINE__);
  memset(ptr, value, num);
  return ptr;
}

// TODO: Needs implemented - ets_timer_arm_new
void ets_timer_arm_new() {
  printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
}

// TODO: Needs implemented - ets_timer_setfn
void ets_timer_setfn() {
  printf("ESPSTUB: ets_timer_setfn: %s: %d\n",__FILE__, __LINE__);
}

// TODO: Needs implemented - ping_start
void ping_start() {
  printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
}

// TODO: Needs implemented - gpio_input_get
void gpio_input_get() {
  printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
}

// TODO: Needs implemented - espconn_port
uint32 espconn_port(void) {
  //printf("ESPSTUB: espconn_port: %s: %d\n",__FILE__, __LINE__);
  return 0;
}

sint8 espconn_send(struct espconn *pEspconn, uint8 *buf, uint16 length) {
  // Send the data through the socket.

  assert(pEspconn != NULL);
  //printf("ESPSTUB: espconn_send: %s: %d\n",__FILE__, __LINE__);
  struct stub_ESP8266Socket *pSocketData = findByEspconn(pEspconn);
  if (pSocketData == NULL) {
    printf("espconn_send: Unable to find socket.");
    return -99;
  }

  // Perform a physical network transmission.
  int rc = eps8266_stub_send(pSocketData, buf, length);
  if (rc == -1) {
    return -99;
  }
  if (pEspconn->sent_callback != NULL) {
    pEspconn->sent_callback(pEspconn);
  }
  return 0;
}

// TODO: Needs implemented - espconn_delete
sint8 espconn_delete(struct espconn *espconn) {
  printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
  return 0;
}

sint8 espconn_disconnect(struct espconn *pEspconn) {
  //printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
  assert(pEspconn != NULL);
  struct stub_ESP8266Socket *pSocket = findByEspconn(pEspconn);
  assert(pSocket != NULL);

  // Call the disconnect function ...
  esp8266_stub_disconnect(pSocket);
  if (pEspconn->proto.tcp->disconnect_callback != NULL) {
    pEspconn->proto.tcp->disconnect_callback(pEspconn);
  }
  releaseSocket(pSocket);
  return 0;
}

sint8 espconn_connect(struct espconn *pEspconn) {
  //printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
  assert(pEspconn != NULL);
  struct stub_ESP8266Socket *pSocket = findByEspconn(pEspconn);
  if (pSocket != NULL) {
    printf("ERROR: espconn_connect: Socket is in use??\n");
    return 1;
  }
  pSocket = getNextFreeSocket();
  pSocket->pEspconn = pEspconn;
  // Execute an actual network connect.
  int rc = esp8266_stub_connect(pSocket);
  if (rc!=0) {
    // We have succeeded in the connect, see if there is a callback to invoke.
    if (pEspconn->proto.tcp->connect_callback != NULL) {
      pEspconn->proto.tcp->connect_callback(pEspconn);
    }
    return 0;
  } else {
    return 1;
  }
}


sint8 espconn_regist_disconcb(struct espconn *pEspconn, espconn_connect_callback discon_cb) {
  //printf("ESPSTUB: espconn_regist_disconcb: %s: %d\n",__FILE__, __LINE__);
  assert(pEspconn != NULL);
  assert(pEspconn->proto.tcp != NULL);
  pEspconn->proto.tcp->disconnect_callback = discon_cb;
  return 0;
}

sint8 espconn_regist_reconcb(struct espconn *espconn, espconn_reconnect_callback recon_cb){
  //printf("ESPSTUB: espconn_regist_reconcb: %s: %d\n",__FILE__, __LINE__);
  assert(espconn != NULL);
  assert(espconn->proto.tcp != NULL);
  espconn->proto.tcp->reconnect_callback = recon_cb;
  return 0;
}

sint8 espconn_regist_sentcb(struct espconn *pEspconn, espconn_sent_callback sent_cb) {
  //printf("ESPSTUB: espconn_regist_sentcb: %s: %d\n",__FILE__, __LINE__);
  assert(pEspconn != NULL);
  pEspconn->sent_callback = sent_cb;
  return 0;
}

sint8 espconn_regist_recvcb(struct espconn *pEspconn, espconn_recv_callback recv_cb){
  //printf("ESPSTUB: espconn_regist_recvcb: %s: %d\n",__FILE__, __LINE__);
  assert(pEspconn != NULL);
  pEspconn->recv_callback = recv_cb;
  return 0;
}

// TODO: Needs implemented - espconn_gethostbyname
err_t espconn_gethostbyname(struct espconn *pEspconn, const char *hostname, ip_addr_t *addr, dns_found_callback found) {
  printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
 return 0;
}

// TODO: Needs implemented - espconn_accept
sint8 espconn_accept(struct espconn *pEspconn) {
  assert(pEspconn != NULL);
  struct stub_ESP8266Socket *pSocket = findByEspconn(pEspconn);
  if (pSocket != NULL) {
    printf("ERROR: espconn_accept: Socket is in use??\n");
    return 1;
  }
  pSocket = getNextFreeSocket();
  pSocket->pEspconn = pEspconn;
  esp8266_stub_listen(pSocket);
  return 0;
}

sint8 espconn_regist_connectcb(struct espconn *pEspconn, espconn_connect_callback connect_cb) {
  //printf("ESPSTUB: espconn_regist_connectcb: %s: %d\n",__FILE__, __LINE__);
  pEspconn->proto.tcp->connect_callback = connect_cb;
  return 0;
}

// TODO: Needs implemented - wifi_softap_free_station_info
void wifi_softap_free_station_info() {
  printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
}

// TODO: Needs implemented - wifi_station_get_rssi
sint8 wifi_station_get_rssi(void) {
  printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
  return 0;
}

uint8 wifi_station_get_connect_status(void) {
  // The return may be one of:
  // o STATION_IDLE
  // o STATION_CONNECTING
  // o STATION_WRONG_PASSWORD
  // o STATION_NO_AP_FOUND
  // o STATION_CONNECT_FAIL
  // o STATION_GOT_IP
  //printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
  return g_wifiStationConnectStatus;
}

bool wifi_station_disconnect(void) {
  //printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
  g_wifiStationConnectStatus = STATION_IDLE;
  return true;
}

// TODO: Needs implemented - system_soft_wdt_feed
void system_soft_wdt_feed() {
  printf("ESPSTUB: %s: %d\n",__FILE__, __LINE__);
}


sint8 espconn_regist_time(struct espconn *pEspconn, uint32 interval, uint8 type_flag) {
  //printf("ESPSTUB: espconn_regist_time: %s: %d\n",__FILE__, __LINE__);
  return 0;
}

sint8 espconn_regist_write_finish(struct espconn *pEspconn, espconn_connect_callback write_finish_fn) {
  //printf("ESPSTUB: espconn_regist_write_finish: %s: %d\n",__FILE__, __LINE__);
  pEspconn->proto.tcp->write_finish_fn = write_finish_fn;
  return 0;
}
