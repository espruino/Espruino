// Over-the-air update HTTP handlers
// By Thorsten von Eicken, 2015

#include <c_types.h>
#include <user_interface.h>
#include <mem.h>
#include <osapi.h>
#include <upgrade.h>
#include <espconn.h>
#include <espmissingincludes.h>
#include "ota.h"

#define OTA_BUFF_SZ 512
#define OTA_CHUNK_SZ 512

// Request handler
struct OtaConn;
typedef int16_t OtaHandler(struct OtaConn *oc);

// Descriptor for an OTA request connection
typedef struct OtaConn {
  struct espconn *conn;
  char           *rxBuffer;     // buffer to accumulate request into
  uint16_t       rxBufFill;     // number of characters in the rxBuffer
  bool           closing;       // just waiting for sent callback to disconnect
  int32_t        rxBufOff;      // offset into req body of first char in buffer, -1:in header
  uint32_t       reqLen;        // length of request body
  OtaHandler     *handler;      // handler to process this request
} OtaConn;

// Format for the response we send
static char *responseFmt =
      "HTTP/1.1 %d %s\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: %d\r\n"
      "Connection: close\r\n"
      "Cache-Control: no-store, no-cache, must-revalidate\r\n"
      "\r\n%s";

/*
 * \brief: releaseConn deallocates everything held by a connection.
 *
 * It doesn't cause any actual communication to appen, so any disconnect call needs to be made
 * separately
 */
static void releaseConn(OtaConn *oc) {
  if (!oc) return;
  if (oc->rxBuffer) os_free(oc->rxBuffer);
  os_memset(oc, 0, sizeof(OtaConn));
}

static void abortConn(OtaConn *oc) {
  struct espconn *conn = oc->conn;
  os_printf("OTA: aborting\n");
  if (conn->reverse != oc) return;
  conn->reverse = NULL;
  espconn_disconnect(conn);
  releaseConn(oc);
}

/*
 * \brief: sendResponse sends an HTTP response with a given status code and null-terminated text
 *
 * This function is very naive because it assumes that the full response can be sent just like
 * that in one go. This is OK because we only send very tiny responses.
 */
static void sendResponse(OtaConn *oc, uint16_t code, char *text) {
  char *status = code < 400 ? "OK" : "ERROR"; // hacky but it works

  // allocate buffer to print the response into
  uint16_t len = os_strlen(responseFmt)+os_strlen(status)+os_strlen(text);
  char buf[len];

  // print the response and send it
  len = os_sprintf(buf, responseFmt, code, status, os_strlen(text), text);
  if (code < 400) os_printf("OTA: %d %s\n", code, status);
  else os_printf("OTA: %d %s <<%s>>\n", code, status, text);
  int8_t err;
  if ((err=espconn_send(oc->conn, (uint8_t*)buf, os_strlen(buf))) != 0) {
    os_printf("OTA: send failed err=%d\n", err);
    abortConn(oc);
  }
}

/**
 * \brief: check_header checks that the header of the firmware blob looks like actual firmware...
 *
 * It returns an error string if something is amiss
 */
static char* check_header(void *buf, int which) {
  uint8_t *cd = (uint8_t *)buf;
  uint32_t *buf32 = buf;
  os_printf("OTA hdr %p: %08lX %08lX %08lX %08lX\n", buf, 
    (long unsigned int)buf32[0], 
    (long unsigned int)buf32[1], 
    (long unsigned int)buf32[2], 
    (long unsigned int)buf32[3]);
  if (cd[0] != 0xEA) return "IROM magic missing";
  if (cd[1] != 4 || cd[2] > 3 || (cd[3]>>4) > 6) return "bad flash header";
  if (cd[3] < 3 && cd[3] != which) return "Wrong partition binary";
  if (((uint16_t *)buf)[3] != 0x4010) return "Invalid entry addr";
  if (((uint32_t *)buf)[2] != 0) return "Invalid start offset";
  return NULL;
}

/**
 * \brief: Handler for the /flash/next request which returns which firmware needs to be uploaded next
 *
 * Sends a response with either "user1.bin" or "user2.bin" as bodies. Returns the number of
 * bytes consumed from the request, which is the full RX buffer.
 */
static int16_t otaHandleNext(OtaConn *oc) {
  uint8 id = system_upgrade_userbin_check();
  char *next = id ? "user1.bin" : "user2.bin";
  sendResponse(oc, 200, next);
  os_printf("OTA: Next firmware: %s (%d=0x%08x)\n", next, id, system_get_userbin_addr());
  return -1; // we're *done*
}

// Information about flash size based on flash memory map as returned by system_get_flash_size_map:
// 4Mb_256, 2Mb, 8Mb_512, 16Mb_512, 32Mb_512, 16Mb_1024, 32Mb_1024
static enum flash_size_map flashSizeMap;

// Start address of user2.bin (address of user1 is always 0x1000)
static uint32_t flashUser2Addr[] = {
  260*1024, 0, 516*1024, 516*1024, 516*1024, 1028*1024, 1028*1024,
};
// Max size of flash firmware
static uint32_t flashMaxSize[] = {
  (256-4-16)*1024, 0,                                // really: too small...
  (512-4-16)*1024, (512-4-16)*1024, (512-4-16)*1024, // 512KB firmware partitions
  (1024-4-16)*1024, (1024-4-16)*1024,                // 1024 KB firmware partitions
};

/*
 * \brief: otaHandleUpload handles a POST to update the firmware
 *
 * Returns the number of bytes consumed from the request, -1 if a response has been sent.
 */
static int16_t otaHandleUpload(OtaConn *oc) {
  uint32_t offset = oc->rxBufOff;

  // assume no error yet...
  char *err = NULL;
  uint16_t code = 400;

  // check overall size
  if (oc->reqLen > flashMaxSize[flashSizeMap]) {
    os_printf("OTA: FW too large: %ld > %ld\n", 
      (long int) oc->reqLen, 
      (long int)  flashMaxSize[flashSizeMap]);
    err = "Firmware image too large";
  } else if (oc->reqLen < OTA_CHUNK_SZ) {
    os_printf("OTA: FW too small: %ld\n", (long int) oc->reqLen);
    err = "Firmware too small";
  }

  // check that we have at least OTA_CHUNK_SZ bytes buffered or it's the last chunk, else we can't write
  if (oc->rxBufFill < OTA_CHUNK_SZ && offset+OTA_CHUNK_SZ <= oc->reqLen) return 0;
  uint16_t toFlash = OTA_CHUNK_SZ;
  if (oc->rxBufFill < toFlash) toFlash = oc->rxBufFill;

  // let's see which partition we need to flash
  uint8 id = system_upgrade_userbin_check();

  // check that data starts with an appropriate header
  if (err == NULL && offset == 0) err = check_header(oc->rxBuffer+oc->rxBufOff, 1-id);

  // return an error if there is one
  if (err != NULL) {
    os_printf("OTA Error %d: %s\n", code, err);
    sendResponse(oc, code, err);
    return -1;
  }

  // let's see which what flash address we're uploading to
  uint32_t address = id ? 0x1000 : flashUser2Addr[flashSizeMap];
  address += offset;

  // erase next flash block if necessary
  if (address % SPI_FLASH_SEC_SIZE == 0){
          os_printf("OTA Flashing 0x%05lx\n", (long unsigned int) address);
          spi_flash_erase_sector(address/SPI_FLASH_SEC_SIZE);
  }

  // write the data
  //os_printf("Writing %d bytes at 0x%05x (%d of %d)\n", connData->post->buffSize, address,
  //		connData->post->received, connData->post->len);
  spi_flash_write(address, (uint32 *)oc->rxBuffer, toFlash);

  if (offset + toFlash == oc->reqLen) {
    sendResponse(oc, 200, "");
    return -1;
  }
  return toFlash;
}

static ETSTimer flash_reboot_timer;

/*
 * \brief: otaRebootFirmware Handle request to reboot into the new firmware
 */
static int16_t otaHandleReboot(OtaConn *oc) {
  // sanity-check that the 'next' partition actually contains something that looks like
  // valid firmware
  uint8 id = system_upgrade_userbin_check();
  uint32_t address = id == 1 ? 0x1000 : flashUser2Addr[flashSizeMap];
  os_printf("OTA: Checking validity at %p\n", (void *)address);

  uint32 buf[8];
  spi_flash_read(address, buf, sizeof(buf));
  char *err = check_header(buf, 1-id);
  if (err != NULL) {
          os_printf("OTA Error %d: %s\n", 400, err);
          sendResponse(oc, 400, err);
          return -1;
  }

  // send empty OK response
  sendResponse(oc, 200, "");

  // Schedule a reboot
  system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
  os_timer_disarm(&flash_reboot_timer);
  os_timer_setfn(&flash_reboot_timer, (os_timer_func_t *)system_upgrade_reboot, NULL);
  os_timer_arm(&flash_reboot_timer, 2000, 1);

  return -1; // we're *done*
}

// Mapping from URLs to request handler functions
#define NUM_HANDLERS 3
static struct {
  char *verb;
  char *path;
  OtaHandler *handler;
} otaHandlers[NUM_HANDLERS] = {
  {"GET", "/flash/next", otaHandleNext },
  {"POST", "/flash/upload", otaHandleUpload },
  {"POST", "/flash/reboot", otaHandleReboot },
};

static OtaHandler *findHandler(char *verb, char *path) {
  uint16_t i=0;
  for (i=0; i<NUM_HANDLERS; i++) {
    if (os_strcmp(verb, otaHandlers[i].verb) == 0 &&
        os_strcmp(path, otaHandlers[i].path) == 0) {
      return otaHandlers[i].handler;
    }
  }
  return NULL;
}

/*
 * \brief: processHeader examines the received buffer and processes the http header if there is one
 *
 * Returns the length of the header, 0 if none is found yet, -1 if an error response has been sent
 */
static int16_t processHeader(OtaConn *oc) {
  // see whether we have the full header by looking for CR-LF-CR-LF
  uint16_t i;
  for (i=0; i<oc->rxBufFill-3; i++) {
    if (oc->rxBuffer[i+0] == '\r' && oc->rxBuffer[i+1] == '\n' &&
        oc->rxBuffer[i+2] == '\r' && oc->rxBuffer[i+3] == '\n') {
      break;
    }
  }
  if (i == oc->rxBufFill-3) return 0; // nope
  oc->rxBuffer[i+3] = 0; // make this null-terminated
  //os_printf("OTA recv: <<%s>>\n", oc->rxBuffer);

  // find the end of the header line
  char *hdrEnd = os_strstr(oc->rxBuffer, "\r\n"); // guaranteed to be found
  *hdrEnd = 0;
  // find start of path/uri
  char *path = os_strchr(oc->rxBuffer, ' ');
  if (!path) {
    sendResponse(oc, 400, "Invalid HTTP request");
    return -1;
  }
  *path++ = 0;
  // find start of http version
  char *vers = os_strchr(path, ' ');
  if (vers) *vers = 0;

  // find handler
  OtaHandler *handler = findHandler(oc->rxBuffer, path);
  if (!handler) {
    sendResponse(oc, 404, "Not found");
    return -1;
  }
  oc->handler = handler;
  os_printf("OTA: %s %s\n", oc->rxBuffer, path);

  // determine content-length
  char *clHdr = os_strstr(hdrEnd+1, "Content-Length:");
  if (clHdr) {
    oc->reqLen = atoi(clHdr+strlen("Content-Length:"));
    os_printf("OTA: content len=%ld\n", (long int) oc->reqLen);
  }

  return i+4;
}

static OtaConn otaConn[1]; // allocate a single connection for now
static struct espconn otaListener; // listening socket
static esp_tcp otaListenerTcp;

/*
 * \brief: Receive data from the HTTP client
 */
static void otaRecvCb(void *arg, char *data, unsigned short len) {
  struct espconn *conn = arg;
  OtaConn *oc = conn->reverse;
  if (!oc || oc != otaConn || conn != otaConn[0].conn) return;
  if (oc->closing) return;

  // allocate a buffer if we have none
  if (oc->rxBuffer == NULL) {
    oc->rxBuffer = os_zalloc(OTA_BUFF_SZ);
    if (!oc->rxBuffer) goto error; // out of memory, disconnect
    oc->rxBufFill = 0;
  }

  int16_t num = 0;
  do {
    if (oc->rxBufFill == OTA_BUFF_SZ) goto error; // buffer full, disconnect

    // copy as much as we can into the rx buffer
    uint16_t cpy = OTA_BUFF_SZ - oc->rxBufFill;
    if (cpy > len) cpy = len;
    os_memcpy(oc->rxBuffer + oc->rxBufFill, data, cpy);
    data += cpy;
    len -= cpy;
    oc->rxBufFill += cpy;

    //os_printf("OTA recv: len=%d fill=%d off=%ld\n", len, oc->rxBufFill, oc->rxBufOff);

    if (oc->rxBufOff < 0 && oc->rxBufFill > 0) {
      // process the request header
      num = processHeader(oc);
      // if we processed the header, shift it out of the buffer
      if (num > 0) {
        os_memmove(oc->rxBuffer, oc->rxBuffer + num, oc->rxBufFill-num);
        oc->rxBufOff = 0;
        oc->rxBufFill -= num;

        // if request has no body, invoke handler here and bail out
        if (oc->reqLen == 0) {
          oc->rxBufFill = 0;
          if (oc->rxBuffer) os_free(oc->rxBuffer);
          oc->rxBuffer = NULL;
          (*oc->handler)(oc);
          return;
        }
      }

    } else if (oc->rxBufOff >= 0 && oc->rxBufFill > 0 && oc->handler) {
      // process request body
      num = (*oc->handler)(oc);
      if (num > 0) {
        os_memmove(oc->rxBuffer, oc->rxBuffer + num, oc->rxBufFill-num);
        oc->rxBufOff += num;
        oc->rxBufFill -= num;
      }

    } else {
      num = 0;
    }

    //os_printf("num=%d\n", num);

    // if we've responded we're ready for the close
    if (num < 0) {
      oc->closing = true;
      return;
    }
  } while (len > 0 || num > 0);

  return;

error:
  abortConn(oc);
}

static void otaDisconCb(void *arg) {
  struct espconn *conn = arg;
  //os_printf("OTA: disconnect\n");
  OtaConn *oc = conn->reverse;
  if (oc) releaseConn(oc);
}

static void otaResetCb(void *arg, sint8 errType) {
  struct espconn *conn = arg;
  os_printf("OTA: conn reset, err=%d\n", errType);
  OtaConn *oc = conn->reverse;
  if (oc) releaseConn(oc);
}

/*
 * \brief: New connection callback
 */
static void otaConnectCb(void *arg)
{
  struct espconn *conn = arg;

  // If the connection is in use, kill it
  if (otaConn[0].conn != NULL) {
    abortConn(otaConn+0);
  }

  os_printf("OTA: connect from ...\n");

  // Initialize new connection
  os_memset(otaConn, 0, sizeof(OtaConn));
  otaConn[0].conn = conn;
  conn->reverse = otaConn+0;
  otaConn[0].rxBufOff = -1; // we start with the http header

  espconn_set_opt(conn, ESPCONN_REUSEADDR|ESPCONN_NODELAY);

  espconn_regist_recvcb(conn, otaRecvCb);
  espconn_regist_disconcb(conn, otaDisconCb);
  espconn_regist_reconcb(conn, otaResetCb);
}

/*
 * \brief: Initialize the OTA server
 */
void otaInit(int port) {
  flashSizeMap = system_get_flash_size_map();

  os_memset(otaConn, 0, sizeof(otaConn));
  os_memset(&otaListener, 0, sizeof(otaListener));
  os_memset(&otaListenerTcp, 0, sizeof(otaListenerTcp));

  // set-up hte listener
  otaListener.type = ESPCONN_TCP;
  otaListener.state = ESPCONN_NONE;
  otaListener.proto.tcp = &otaListenerTcp;
  otaListenerTcp.local_port = port;

  espconn_regist_connectcb(&otaListener, otaConnectCb);
  espconn_accept(&otaListener);
  espconn_tcp_set_max_con_allow(&otaListener, 2); // kill first conn if there is a 2nd
  espconn_regist_time(&otaListener, 30, 0);
}
