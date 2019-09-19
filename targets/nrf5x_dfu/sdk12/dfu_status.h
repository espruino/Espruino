typedef enum {
  DFUS_ADVERTISING_START,
  DFUS_ADVERTISING_STOP,
  DFUS_CONNECTED,
  DFUS_DISCONNECTED,
} DFUStatus;

extern void dfu_set_status(DFUStatus status);
