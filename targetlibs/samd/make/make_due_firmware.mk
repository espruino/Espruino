#-------------------------------------------------------------------------------
# @author stfwi
# @file make_due_firmware.mk
#
# INCLUDE THIS FILE IN YOUR MAKEFILE USING THE ...
#
#   include <path/to/>make_due_firmware.mk
#
# ... MAKEFILE DIRECTIVE.
#
# You have then the options:
#
#   make all	  : Everything except uploading
#   make binary	  : Makes the binary that can be uploaded (default firmware.bin)
#   make install  : Uploads to the DUE using BOSSA
#   make clean	  : Cleans up the "$OUTPUT_DIR/release" or "$OUTPUT_DIR/debug"
#
#-------------------------------------------------------------------------------


#-------------------------------------------------------------------------------
# CONFIG: OVERWRITE as params to modify
#-------------------------------------------------------------------------------

.SUFFIXES: .o .a .c .s
SHELL = /bin/sh
OUTPUT_NAME=firmware
OUTPUT_DIR = .
PROJECT_DIR = .
UPLOAD_PORT=/dev/cu.usbmodemfa141
USB_DEFINITIONS=-DUSB_VID=0x2341 -DUSB_PID=0x003e -DUSBCON '-DUSB_MANUFACTURER="Unknown"' '-DUSB_PRODUCT="Arduino Due"'

#-------------------------------------------------------------------------------
# Related directories and files
#-------------------------------------------------------------------------------
ROOT := $(realpath $(shell dirname '$(dir $(lastword $(MAKEFILE_LIST)))'))
vpath %.c $(PROJECT_DIR)
VPATH+=$(PROJECT_DIR)

INCLUDES =
INCLUDES += -I$(ROOT)/include
INCLUDES += -I$(ROOT)/sam
INCLUDES += -I$(ROOT)/sam/libsam
INCLUDES += -I$(ROOT)/sam/CMSIS/CMSIS/Include
INCLUDES += -I$(ROOT)/sam/CMSIS/Device/ATMEL

#-------------------------------------------------------------------------------
# Target selection
#-------------------------------------------------------------------------------
ifdef DEBUG
OPTIMIZATION = -g -O0 -DDEBUG
OBJ_DIR=debug
else
OPTIMIZATION = -Os
OBJ_DIR=release
endif

#-------------------------------------------------------------------------------
#  Toolchain
#-------------------------------------------------------------------------------
CROSS_COMPILE = $(ROOT)/tools/g++_arm_none_eabi/bin/arm-none-eabi-
AR = $(CROSS_COMPILE)ar
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AS = $(CROSS_COMPILE)as
NM = $(CROSS_COMPILE)nm
LKELF = $(CROSS_COMPILE)g++
OBJCP = $(CROSS_COMPILE)objcopy
RM=rm -Rf
MKDIR=mkdir -p
UPLOAD_BOSSA=$(ROOT)/tools/bossac


#-------------------------------------------------------------------------------
#  Flags
#-------------------------------------------------------------------------------
CFLAGS += -Wall --param max-inline-insns-single=500 -mcpu=cortex-m3 -mthumb -mlong-calls
CFLAGS += -ffunction-sections -fdata-sections -nostdlib -std=c99
CFLAGS += $(OPTIMIZATION) $(INCLUDES)
#CFLAGS += -Dprintf=iprintf
CPPFLAGS += -Wall --param max-inline-insns-single=500 -mcpu=cortex-m3 -mthumb -mlong-calls -nostdlib
CPPFLAGS += -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -std=c++98
CPPFLAGS += $(OPTIMIZATION) $(INCLUDES)
#CPPFLAGS += -Dprintf=iprintf
ASFLAGS = -mcpu=cortex-m3 -mthumb -Wall -g $(OPTIMIZATION) $(INCLUDES)
ARFLAGS = rcs

LNK_SCRIPT=$(ROOT)/sam/linker_scripts/gcc/flash.ld
LIBSAM_ARCHIVE=$(ROOT)/lib/libsam_sam3x8e_gcc_rel.a

UPLOAD_PORT_BASENAME=$(patsubst /dev/%,%,$(UPLOAD_PORT))

#-------------------------------------------------------------------------------
# High verbosity flags
#-------------------------------------------------------------------------------
ifdef VERBOSE
CFLAGS += -Wall -Wchar-subscripts -Wcomment -Wformat=2 -Wimplicit-int
CFLAGS += -Werror-implicit-function-declaration -Wmain -Wparentheses
CFLAGS += -Wsequence-point -Wreturn-type -Wswitch -Wtrigraphs -Wunused
CFLAGS += -Wuninitialized -Wunknown-pragmas -Wfloat-equal -Wundef
CFLAGS += -Wshadow -Wpointer-arith -Wbad-function-cast -Wwrite-strings
CFLAGS += -Wsign-compare -Waggregate-return -Wstrict-prototypes
CFLAGS += -Wmissing-prototypes -Wmissing-declarations
CFLAGS += -Wformat -Wmissing-format-attribute -Wno-deprecated-declarations
CFLAGS += -Wredundant-decls -Wnested-externs -Winline -Wlong-long
CFLAGS += -Wunreachable-code
CFLAGS += -Wcast-align
CFLAGS += -Wmissing-noreturn
CFLAGS += -Wconversion
CPPFLAGS += -Wall -Wchar-subscripts -Wcomment -Wformat=2
CPPFLAGS += -Wmain -Wparentheses -Wcast-align -Wunreachable-code
CPPFLAGS += -Wsequence-point -Wreturn-type -Wswitch -Wtrigraphs -Wunused
CPPFLAGS += -Wuninitialized -Wunknown-pragmas -Wfloat-equal -Wundef
CPPFLAGS += -Wshadow -Wpointer-arith -Wwrite-strings
CPPFLAGS += -Wsign-compare -Waggregate-return -Wmissing-declarations
CPPFLAGS += -Wformat -Wmissing-format-attribute -Wno-deprecated-declarations
CPPFLAGS += -Wpacked -Wredundant-decls -Winline -Wlong-long
CPPFLAGS += -Wmissing-noreturn
CPPFLAGS += -Wconversion
UPLOAD_VERBOSE_FLAGS += -i -d
endif

#-------------------------------------------------------------------------------
# Source files and objects
#-------------------------------------------------------------------------------
C_SRC=$(wildcard $(PROJECT_DIR)/*.c) $(wildcard ../src/*.c)
C_OBJ=$(patsubst %.c, %.o, $(notdir $(C_SRC)))
CPP_SRC=$(wildcard $(PROJECT_DIR)/*.cpp)
CPP_OBJ=$(patsubst %.cpp, %.o, $(notdir $(CPP_SRC)))
A_SRC=$(wildcard $(PROJECT_DIR)/*.s)
A_OBJ=$(patsubst %.s, %.o, $(notdir $(A_SRC)))

#-------------------------------------------------------------------------------
# Rules
#-------------------------------------------------------------------------------


#-------------------------------------------------------------------------------
all: binary

#-------------------------------------------------------------------------------
.PHONY: clean
clean:
	-@$(RM) $(OBJ_DIR) 1>/dev/null 2>&1
	-@$(RM) $(OUTPUT_DIR)/$(OUTPUT_NAME) 1>/dev/null 2>&1

#-------------------------------------------------------------------------------
.PHONY: prepare
prepare:
	-@$(MKDIR) $(OBJ_DIR) 1>/dev/null 2>&1

#-------------------------------------------------------------------------------
.PHONY: binary
binary: prepare $(OBJ_DIR)/$(OUTPUT_NAME).bin

#-------------------------------------------------------------------------------
# .bin ------> UPLOAD TO CONTROLLER
.PHONY: install
install: binary
	-@echo "Touch programming port ..."
	-@stty -f "/dev/$(UPLOAD_PORT_BASENAME)" raw ispeed 1200 ospeed 1200 cs8 -cstopb ignpar eol 255 eof 255
	-@printf "\x00" > "/dev/$(UPLOAD_PORT_BASENAME)"
	-@echo "Waiting before uploading ..."
	-@sleep 1
	-@echo "Uploading ..."
#	$(UPLOAD_BOSSA) $(UPLOAD_VERBOSE_FLAGS) --port="$(UPLOAD_PORT_BASENAME)" -U false -e -w -b "$(OBJ_DIR)/$(OUTPUT_NAME).bin" -R
	$(UPLOAD_BOSSA) $(UPLOAD_VERBOSE_FLAGS) --port="$(UPLOAD_PORT_BASENAME)" -U false -e -w -v -b "$(OBJ_DIR)/$(OUTPUT_NAME).bin" -R
	@echo "Done."

#-------------------------------------------------------------------------------
# .c -> .o
$(addprefix $(OBJ_DIR)/,$(C_OBJ)): $(OBJ_DIR)/%.o: %.c
	"$(CC)" -c $(CFLAGS) $< -o $@

# .cpp -> .o
$(addprefix $(OBJ_DIR)/,$(CPP_OBJ)): $(OBJ_DIR)/%.o: %.cpp
	"$(CC)" -xc++ -c $(CPPFLAGS) $< -o $@

# .s -> .o
$(addprefix $(OBJ_DIR)/,$(A_OBJ)): $(OBJ_DIR)/%.o: %.s
	"$(AS)" -c $(ASFLAGS) $< -o $@

# .o -> .a
$(OBJ_DIR)/$(OUTPUT_NAME).a: $(addprefix $(OBJ_DIR)/, $(C_OBJ)) $(addprefix $(OBJ_DIR)/, $(CPP_OBJ)) $(addprefix $(OBJ_DIR)/, $(A_OBJ))
	"$(AR)" $(ARFLAGS) $@ $^
	"$(NM)" $@ > $@.txt

#  -> .elf
$(OBJ_DIR)/$(OUTPUT_NAME).elf: $(OBJ_DIR)/$(OUTPUT_NAME).a
	"$(LKELF)" -Os -Wl,--gc-sections -mcpu=cortex-m3 \
	  "-T$(LNK_SCRIPT)" "-Wl,-Map,$(OBJ_DIR)/$(OUTPUT_NAME).map" \
	  -o $@ \
	  "-L$(OBJ_DIR)" \
	  -lm -lgcc -mthumb -Wl,--cref -Wl,--check-sections -Wl,--gc-sections \
	  -Wl,--entry=Reset_Handler -Wl,--unresolved-symbols=report-all -Wl,--warn-common \
	  -Wl,--warn-section-align -Wl,--warn-unresolved-symbols \
	  -Wl,--start-group \
	  $^ $(LIBSAM_ARCHIVE) \
	  -Wl,--end-group

# .elf -> .bin
$(OBJ_DIR)/$(OUTPUT_NAME).bin: $(OBJ_DIR)/$(OUTPUT_NAME).elf
	"$(OBJCP)" -O binary $< $@
