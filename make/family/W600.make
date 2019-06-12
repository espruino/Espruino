W600=1

ARM_HAS_OWN_CMSIS=1


CFLAGS  += -Wno-sign-conversion -Wno-conversion -Wno-unused-parameter -fomit-frame-pointer -std=gnu99 -fgnu89-inline \
           -mthumb -mthumb-interwork -Os -mcpu=cortex-m3 -march=armv7-m -mabi=aapcs -Wno-unused-function -Wno-attributes 
           
INCLUDE +=  -I${SDK_ROOT}/platform/boot/gcc \
            -I${SDK_ROOT}/platform/inc \
            -I${SDK_ROOT}/platform/common/crypto \
            -I${SDK_ROOT}/platform/common/crypto/digest \
            -I${SDK_ROOT}/platform/common/crypto/math \
            -I${SDK_ROOT}/platform/common/crypto/symmetric \
            -I${SDK_ROOT}/platform/common/crypto/pubkey \
            -I${SDK_ROOT}/platform/common/params \
            -I${SDK_ROOT}/platform/sys \
            -I${SDK_ROOT}/include \
            -I${SDK_ROOT}/include/driver \
            -I${SDK_ROOT}/include/os \
            -I${SDK_ROOT}/include/net \
            -I${SDK_ROOT}/include/app \
            -I${SDK_ROOT}/include/wifi \
            -I${SDK_ROOT}/include/platform \
            -I${SDK_ROOT}/src/os/rtos/include \
            -I${SDK_ROOT}/src/app/matrixssl \
            -I${SDK_ROOT}/src/app/dhcpserver \
            -I${SDK_ROOT}/src/app/ping  \
            -I${SDK_ROOT}/src/app/iperf  \
            -I${SDK_ROOT}/src/app/dnsserver  \
            -I${SDK_ROOT}/src/network/lwip2.0.3/include  \
            -I${SDK_ROOT}/src/network/api \
            -I${ROOT}/targets/w600   

SOURCES +=  ${SDK_ROOT}/platform/boot/gcc/startup.s \
            ${SDK_ROOT}/platform/boot/gcc/misc.c \
            ${SDK_ROOT}/platform/sys/tls_sys.c \
            ${ROOT}/targets/w600/retarget_sub.c \
            ${ROOT}/targets/w600/jshardware.c \
            ${ROOT}/targets/w600/user_main.c \
            ${ROOT}/targets/w600/wm_main.c 


DEFINES += -DGCC_COMPILE \
           -DW600 \
           -DUSE_OS 

LIBS += ${SDK_ROOT}/lib/libwlan.a \
        ${SDK_ROOT}/lib/libnetwork.a \
        ${SDK_ROOT}/lib/libcommon.a \
        ${SDK_ROOT}/lib/libdrivers.a \
        ${SDK_ROOT}/lib/libapp.a \
        ${SDK_ROOT}/lib/libairkiss.a \
        ${SDK_ROOT}/lib/libos.a

LINKER_FILE = ${SDK_ROOT}/ld/w600_1m.ld

LDFLAGS += -static -nostartfiles -mthumb -mcpu=cortex-m3

include make/common/ARM.make

export CCPREFIX=arm-none-eabi-

