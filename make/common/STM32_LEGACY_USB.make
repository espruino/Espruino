# Old Legacy STM32 USB
# Used for F1 and F3

STM32_LEGACY_USB=1

DEFINES += -DLEGACY_USB
INCLUDE += -I$(ROOT)/targetlibs/stm32legacyusb/lib -I$(ROOT)/targetlibs/stm32legacyusb
TARGETSOURCES +=                              \
targetlibs/stm32legacyusb/lib/otgd_fs_cal.c       \
targetlibs/stm32legacyusb/lib/otgd_fs_dev.c       \
targetlibs/stm32legacyusb/lib/otgd_fs_int.c       \
targetlibs/stm32legacyusb/lib/otgd_fs_pcd.c       \
targetlibs/stm32legacyusb/lib/usb_core.c          \
targetlibs/stm32legacyusb/lib/usb_init.c          \
targetlibs/stm32legacyusb/lib/usb_int.c           \
targetlibs/stm32legacyusb/lib/usb_mem.c           \
targetlibs/stm32legacyusb/lib/usb_regs.c          \
targetlibs/stm32legacyusb/lib/usb_sil.c           \
targetlibs/stm32legacyusb/usb_desc.c             \
targetlibs/stm32legacyusb/usb_endp.c             \
targetlibs/stm32legacyusb/usb_istr.c             \
targetlibs/stm32legacyusb/usb_prop.c             \
targetlibs/stm32legacyusb/usb_pwr.c              \
targetlibs/stm32legacyusb/usb_utils.c            \
targetlibs/stm32legacyusb/legacy_usb.c
