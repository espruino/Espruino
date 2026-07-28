# Makefile for the Raspberry Pi RP2350 family, built against the Pico SDK
# (PICO_SDK_PATH must point at it, e.g. ~/.pico-sdk/sdk/2.1.1).
# PICO_BOARD selects the Pico SDK board header (default pico2), eg:
#   PICO_BOARD=pico2_w BOARD=RP2350 make

RP2350=1

ifndef PICO_SDK_PATH
$(error PICO_SDK_PATH is not set)
endif
PICO_SDK_SRC = $(PICO_SDK_PATH)/src

PICO_BOARD ?= pico2
PROJ_NAME := $(PROJ_NAME)_$(PICO_BOARD)

ARCHFLAGS += -mcpu=cortex-m33 -mthumb -mfloat-abi=softfp -mfpu=fpv5-sp-d16

DEFINES += -DPICO_RP2350=1
DEFINES += -DPICO_STACK_SIZE=0x1000 # Espruino needs more than the SDK's 2KB default

INCLUDE += \
  -I$(PICO_SDK_SRC)/rp2350/hardware_regs/include \
  -I$(PICO_SDK_SRC)/rp2350/hardware_structs/include \
  -I$(PICO_SDK_SRC)/rp2350/pico_platform/include \
  -I$(PICO_SDK_SRC)/rp2_common/hardware_base/include \
  -I$(PICO_SDK_SRC)/rp2_common/hardware_gpio/include \
  -I$(PICO_SDK_SRC)/rp2_common/hardware_uart/include \
  -I$(PICO_SDK_SRC)/rp2_common/hardware_adc/include \
  -I$(PICO_SDK_SRC)/rp2_common/hardware_flash/include \
  -I$(PICO_SDK_SRC)/rp2_common/hardware_clocks/include \
  -I$(PICO_SDK_SRC)/rp2_common/hardware_pll/include \
  -I$(PICO_SDK_SRC)/rp2_common/hardware_xosc/include \
  -I$(PICO_SDK_SRC)/rp2_common/hardware_watchdog/include \
  -I$(PICO_SDK_SRC)/rp2_common/hardware_vreg/include \
  -I$(PICO_SDK_SRC)/rp2_common/hardware_resets/include \
  -I$(PICO_SDK_SRC)/rp2_common/hardware_irq/include \
  -I$(PICO_SDK_SRC)/rp2_common/hardware_sync/include \
  -I$(PICO_SDK_SRC)/rp2_common/hardware_sync_spin_lock/include \
  -I$(PICO_SDK_SRC)/rp2_common/hardware_boot_lock/include \
  -I$(PICO_SDK_SRC)/rp2_common/hardware_timer/include \
  -I$(PICO_SDK_SRC)/rp2_common/hardware_ticks/include \
  -I$(PICO_SDK_SRC)/rp2_common/hardware_xip_cache/include \
  -I$(PICO_SDK_SRC)/rp2_common/pico_platform_compiler/include \
  -I$(PICO_SDK_SRC)/rp2_common/pico_platform_sections/include \
  -I$(PICO_SDK_SRC)/rp2_common/pico_platform_panic/include \
  -I$(PICO_SDK_SRC)/rp2_common/pico_runtime/include \
  -I$(PICO_SDK_SRC)/rp2_common/pico_runtime_init/include \
  -I$(PICO_SDK_SRC)/rp2_common/pico_clib_interface/include \
  -I$(PICO_SDK_SRC)/rp2_common/pico_bootrom/include \
  -I$(PICO_SDK_SRC)/rp2_common/pico_time_adapter/include \
  -I$(PICO_SDK_SRC)/rp2_common/pico_unique_id/include \
  -I$(PICO_SDK_SRC)/rp2_common/pico_flash/include \
  -I$(PICO_SDK_SRC)/rp2_common/boot_bootrom_headers/include \
  -I$(PICO_SDK_SRC)/common/pico_base_headers/include \
  -I$(PICO_SDK_SRC)/common/pico_time/include \
  -I$(PICO_SDK_SRC)/common/pico_sync/include \
  -I$(PICO_SDK_SRC)/common/pico_util/include \
  -I$(PICO_SDK_SRC)/common/pico_binary_info/include \
  -I$(PICO_SDK_SRC)/common/hardware_claim/include \
  -I$(PICO_SDK_SRC)/common/boot_picoboot_headers/include \
  -I$(PICO_SDK_SRC)/common/boot_picobin_headers/include \
  -I$(PICO_SDK_SRC)/rp2350/boot_stage2/asminclude \
  -I$(PICO_SDK_SRC)/rp2350/boot_stage2/include \
  -I$(PICO_SDK_SRC)/boards/include \
  -I$(ROOT)/targets/rp2350 \
  -I$(GENDIR)

# --- Generated pico/version.h + pico/config_autogen.h (normally made by the SDK's CMake) ---
PICO_SDK_VERSION := $(shell sed -n 's/^set(PICO_SDK_VERSION_[A-Z]* \([0-9A-Za-z]*\))/\1/p' $(PICO_SDK_PATH)/pico_sdk_version.cmake | head -3 | paste -sd.)
$(GENDIR)/pico/version.h: $(PICO_SDK_PATH)/pico_sdk_version.cmake
	@mkdir -p $(GENDIR)/pico
	@sed -n 's/^set(PICO_SDK_\(VERSION_[A-Z]*\) \([0-9]*\))/#define PICO_SDK_\1 \2/p' $< > $@
	@echo '#define PICO_SDK_VERSION_STRING "$(PICO_SDK_VERSION)"' >> $@

# Stamp file so config_autogen.h (and everything built against it) rebuilds when PICO_BOARD changes
PICO_BOARD_STAMP = $(GENDIR)/.pico_board
FORCE:
$(PICO_BOARD_STAMP): FORCE
	@mkdir -p $(GENDIR)
	@echo $(PICO_BOARD) | cmp -s $@ - 2>/dev/null || echo $(PICO_BOARD) > $@

$(GENDIR)/pico/config_autogen.h: $(PICO_BOARD_STAMP)
	@mkdir -p $(GENDIR)/pico
	@echo '#include "boards/$(PICO_BOARD).h"' > $@

$(PLATFORM_CONFIG_FILE): $(GENDIR)/pico/config_autogen.h $(GENDIR)/pico/version.h

# --- Source files ---
TARGETSOURCES += targets/rp2350/jshardware.c \
                 targets/rp2350/main.c

# Pico SDK crt0 provides the vector table, IMAGE_DEF block, reset handler and
# entry point; boot_stage2 provides the flash/XIP setup the bootrom runs first.
CRT0_OBJ = $(OBJDIR)/$(PICO_SDK_SRC)/rp2_common/pico_crt0/crt0.o
BOOT2_OBJ = $(OBJDIR)/$(PICO_SDK_SRC)/rp2350/boot_stage2/compile_time_choice.o
IRQCHAIN_OBJ = $(OBJDIR)/$(PICO_SDK_SRC)/rp2_common/hardware_irq/irq_handler_chain.o
PRECOMPILED_OBJS += $(CRT0_OBJ) $(BOOT2_OBJ) $(IRQCHAIN_OBJ)

# Order-only prerequisite: assembly includes the generated pico/ headers
GEN_PICO_HEADERS = $(GENDIR)/pico/version.h $(GENDIR)/pico/config_autogen.h

$(CRT0_OBJ): $(PICO_SDK_SRC)/rp2_common/pico_crt0/crt0.S | $(GEN_PICO_HEADERS)
	@echo AS $@
	@mkdir -p $(shell dirname $@)
	@$(CC) -c $(CFLAGS) -DPICO_EMBED_XIP_SETUP=0 -I$(PICO_SDK_SRC)/rp2_common/pico_crt0 -x assembler-with-cpp $< -o $@

# Rename .text to .boot2 so the linker script places it as the boot stage 2
$(BOOT2_OBJ): $(PICO_SDK_SRC)/rp2350/boot_stage2/compile_time_choice.S | $(GEN_PICO_HEADERS)
	@echo AS $@
	@mkdir -p $(shell dirname $@)
	@$(CC) -c $(CFLAGS) -x assembler-with-cpp $< -o $@.tmp
	@arm-none-eabi-objcopy --rename-section .text=.boot2,contents,alloc,load,readonly,code $@.tmp $@
	@rm -f $@.tmp

$(IRQCHAIN_OBJ): $(PICO_SDK_SRC)/rp2_common/hardware_irq/irq_handler_chain.S | $(GEN_PICO_HEADERS)
	@echo AS $@
	@mkdir -p $(shell dirname $@)
	@$(CC) -c $(CFLAGS) -x assembler-with-cpp $< -o $@

# Pico SDK hardware drivers + runtime (only what's needed - add more as jshardware.c grows)
TARGETSOURCES += \
  $(PICO_SDK_SRC)/rp2_common/hardware_gpio/gpio.c \
  $(PICO_SDK_SRC)/rp2_common/hardware_uart/uart.c \
  $(PICO_SDK_SRC)/rp2_common/hardware_adc/adc.c \
  $(PICO_SDK_SRC)/rp2_common/hardware_flash/flash.c \
  $(PICO_SDK_SRC)/rp2_common/hardware_clocks/clocks.c \
  $(PICO_SDK_SRC)/rp2_common/hardware_pll/pll.c \
  $(PICO_SDK_SRC)/rp2_common/hardware_xosc/xosc.c \
  $(PICO_SDK_SRC)/rp2_common/hardware_watchdog/watchdog.c \
  $(PICO_SDK_SRC)/rp2_common/hardware_irq/irq.c \
  $(PICO_SDK_SRC)/rp2_common/hardware_timer/timer.c \
  $(PICO_SDK_SRC)/rp2_common/hardware_ticks/ticks.c \
  $(PICO_SDK_SRC)/rp2_common/hardware_sync/sync.c \
  $(PICO_SDK_SRC)/rp2_common/hardware_sync_spin_lock/sync_spin_lock.c \
  $(PICO_SDK_SRC)/rp2_common/hardware_xip_cache/xip_cache.c \
  $(PICO_SDK_SRC)/common/hardware_claim/claim.c \
  $(PICO_SDK_SRC)/rp2_common/pico_runtime/runtime.c \
  $(PICO_SDK_SRC)/rp2_common/pico_runtime_init/runtime_init.c \
  $(PICO_SDK_SRC)/rp2_common/pico_runtime_init/runtime_init_clocks.c \
  $(PICO_SDK_SRC)/rp2_common/pico_runtime_init/runtime_init_stack_guard.c \
  $(PICO_SDK_SRC)/rp2350/pico_platform/platform.c \
  $(PICO_SDK_SRC)/rp2_common/pico_platform_panic/panic.c \
  $(PICO_SDK_SRC)/rp2_common/pico_clib_interface/newlib_interface.c \
  $(PICO_SDK_SRC)/rp2_common/pico_bootrom/bootrom.c \
  $(PICO_SDK_SRC)/rp2_common/pico_unique_id/unique_id.c \
  $(PICO_SDK_SRC)/common/pico_sync/mutex.c \
  $(PICO_SDK_SRC)/common/pico_sync/critical_section.c \
  $(PICO_SDK_SRC)/common/pico_sync/lock_core.c \
  $(PICO_SDK_SRC)/common/pico_time/time.c \
  $(PICO_SDK_SRC)/common/pico_time/timeout_helper.c

# --- Linker ---
# Use the Pico SDK's linker script 
# The FLASH MEMORY region it INCLUDEs is supplied by targets/rp2350/pico_flash_region.ld via -L.
LINKER_FILE = $(PICO_SDK_SRC)/rp2_common/pico_crt0/rp2350/memmap_default.ld
LDFLAGS += -L$(ROOT)/targets/rp2350
# The SDK's pad_checksum step (which we skip) normally emits __boot2_entry_point.
# Alias it to __boot2_start__ - the section start, NOT _stage2_boot whose Thumb bit would fault crt0's word-aligned load.
LDFLAGS += -Wl,--defsym=__boot2_entry_point=__boot2_start__
# Keep the SDK's weak _fstat/_isatty stubs: only non-LTO newlib stdio (via FILESYSTEM) uses them, so LTO would drop them
LDFLAGS += -Wl,-u,_fstat -Wl,-u,_isatty
LDFLAGS += $(OPTIMIZEFLAGS) $(ARCHFLAGS) --specs=nano.specs -lc -lnosys

$(PROJ_NAME).uf2: $(PROJ_NAME).elf
	@echo Creating $@
	$(Q)arm-none-eabi-objcopy -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin
	$(Q)python3 $(ROOT)/scripts/uf2/uf2conv.py $(PROJ_NAME).bin -b 0x10000000 -f 0xe48bff59 -o $(PROJ_NAME).uf2

proj: $(PROJ_NAME).uf2

include make/common/ARM.make
