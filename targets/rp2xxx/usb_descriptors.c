#include <string.h>

#include "pico/unique_id.h"
#include "tusb.h"

#define USBD_VID 0x2E8A
#define USBD_PID 0x000A
#define USBD_CONFIGURATION_DESCRIPTOR_ATTRIBUTE 0
#define USBD_MAX_POWER_MA 250
#define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

#define USBD_STR_0 0x00
#define USBD_STR_MANUF 0x01
#define USBD_STR_PRODUCT 0x02
#define USBD_STR_SERIAL 0x03
#define USBD_STR_CDC 0x04

static const tusb_desc_device_t usbd_desc_device = {
  .bLength = sizeof(tusb_desc_device_t),
  .bDescriptorType = TUSB_DESC_DEVICE,
  .bcdUSB = 0x0210,
  .bDeviceClass = TUSB_CLASS_MISC,
  .bDeviceSubClass = MISC_SUBCLASS_COMMON,
  .bDeviceProtocol = MISC_PROTOCOL_IAD,
  .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
  .idVendor = USBD_VID,
  .idProduct = USBD_PID,
  .bcdDevice = 0x0100,
  .iManufacturer = USBD_STR_MANUF,
  .iProduct = USBD_STR_PRODUCT,
  .iSerialNumber = USBD_STR_SERIAL,
  .bNumConfigurations = 1,
};

static const uint8_t usbd_desc_cfg[USBD_DESC_LEN] = {
  TUD_CONFIG_DESCRIPTOR(1, USBD_ITF_MAX, USBD_STR_0, USBD_DESC_LEN,
    USBD_CONFIGURATION_DESCRIPTOR_ATTRIBUTE, USBD_MAX_POWER_MA),
  TUD_CDC_DESCRIPTOR(USBD_ITF_CDC, USBD_STR_CDC, USBD_CDC_EP_CMD,
    USBD_CDC_CMD_MAX_SIZE, USBD_CDC_EP_OUT, USBD_CDC_EP_IN, USBD_CDC_IN_OUT_MAX_SIZE),
};

static char usbd_serial_str[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];

static const char *const usbd_desc_str[] = {
  [USBD_STR_MANUF] = "Raspberry Pi",
  [USBD_STR_PRODUCT] = "Pico",
  [USBD_STR_SERIAL] = usbd_serial_str,
  [USBD_STR_CDC] = "Board CDC",
};

uint8_t const *tud_descriptor_device_cb(void) {
  return (const uint8_t *)&usbd_desc_device;
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
  (void)index;
  return usbd_desc_cfg;
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void)langid;
  static uint16_t desc_str[20];

  if (!usbd_serial_str[0]) {
    pico_get_unique_board_id_string(usbd_serial_str, sizeof(usbd_serial_str));
  }

  uint8_t len;
  if (index == 0) {
    desc_str[1] = 0x0409;
    len = 1;
  } else {
    if (index >= sizeof(usbd_desc_str) / sizeof(usbd_desc_str[0])) return NULL;
    const char *str = usbd_desc_str[index];
    if (!str) return NULL;
    for (len = 0; len < (sizeof(desc_str) / sizeof(desc_str[0])) - 1 && str[len]; ++len) {
      desc_str[1 + len] = str[len];
    }
  }

  desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * len + 2));
  return desc_str;
}
