#!/bin/bash

# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2017 Gordon Williams <gw@pur3.co.uk>
# wilberforce (Rhys Williams)
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# -----------------------------------------------------------------------------
# Setup toolchain and libraries for build targets, installs if missing
# set env vars for builds
# For use in:
#    Travis
#    Firmware builds
#    Docker
#
# -----------------------------------------------------------------------------

if [ $# -eq 0 ]
then
  echo "USAGE:"
  echo "source scripts/provision.sh {BOARD}"
  return 1
fi

# set the current board
BOARDNAME=$1
FAMILY=`scripts/get_board_info.py $BOARDNAME 'board.chip["family"]'`

if [ $FAMILY = "ESP32" ]; then
    echo ESP32
    if [ ! -d "app" ]; then
        echo installing app folder
        curl -Ls https://github.com/espruino/EspruinoBuildTools/raw/master/esp32/deploy/app.tgz | tar xfz -
    fi
    if [ ! -d "esp-idf" ]; then
        echo installing esp-idf folder
        curl -Ls https://github.com/espruino/EspruinoBuildTools/raw/master/esp32/deploy/esp-idf.tgz | tar xfz -
    fi
    if ! type xtensa-esp32-elf-gcc > /dev/null; then
        echo installing xtensa-esp32-elf-gcc
        if [ ! -d "xtensa-esp32-elf" ]; then
           curl -Ls https://dl.espressif.com/dl/xtensa-esp32-elf-linux64-1.22.0-61-gab8375a-5.2.0.tar.gz | tar xfz -
        else
           echo "Folder found"
        fi
    fi
    which xtensa-esp32-elf-gcc
    export ESP_IDF_PATH=`pwd`/esp-idf
    export ESP_APP_TEMPLATE_PATH=`pwd`/app
    export PATH=$PATH:`pwd`/xtensa-esp32-elf/bin/
    return 0
elif [ $FAMILY = "ESP8266" ]; then
    echo ESP8266
    if [ ! -d "esp_iot_sdk_v2.0.0.p1" ]; then
        echo esp_iot_sdk_v2.0.0.p1
        curl -Ls http://s3.voneicken.com/esp_iot_sdk_v2.0.0.p1.tgx | tar Jxf -
    fi
    if ! type xtensa-lx106-elf-gcc > /dev/null; then
        echo installing xtensa-lx106-elf-gcc
        if [ ! -d "xtensa-lx106-elf" ]; then
            curl -Ls http://s3.voneicken.com/xtensa-lx106-elf-20160330.tgx | tar Jxf -
        else
            echo "Folder found"
        fi

    fi
    which xtensa-lx106-elf-gcc
    export ESP8266_SDK_ROOT=`pwd`/esp_iot_sdk_v2.0.0.p1
    export PATH=$PATH:`pwd`/xtensa-lx106-elf/bin/
    return 0
elif [ $FAMILY = "LINUX" ]; then
    echo LINUX
    # Raspberry Pi?
    return 0
elif [ $FAMILY = "NRF52" ]; then
    echo NRF52
    if ! type nrfutil > /dev/null; then
      echo Installing nrfutil
      sudo apt-get install -y python python-pip
      sudo pip install nrfutil
    fi
    ARM=1
elif [ $FAMILY = "NRF51" ]; then
    ARM=1
elif [ $FAMILY = "STM32F1" ]; then
    ARM=1
elif [ $FAMILY = "STM32F3" ]; then
    ARM=1
elif [ $FAMILY = "STM32F4" ]; then
    ARM=1
elif [ $FAMILY = "STM32L4" ]; then
    ARM=1
elif [ $FAMILY = "EFM32GG" ]; then
    ARM=1
else
    echo "Unknown board ($BOARDNAME) or family ($FAMILY)"
    return 1
fi

if [ $ARM = "1" ]; then
    # defaulting to ARM
    echo ARM
    if ! type arm-none-eabi-gcc > /dev/null; then
        echo installing gcc-arm-embedded
        #sudo add-apt-repository -y ppa:team-gcc-arm-embedded/ppa
        #sudo apt-get update
        #sudo apt-get --force-yes --yes install libsdl1.2-dev gcc-arm-embedded
        # Unpack - newer, and much faster
        if [ ! -d "gcc-arm-none-eabi-6-2017-q1-update" ]; then
          curl -Ls https://github.com/espruino/EspruinoBuildTools/raw/master/arm/gcc-arm-none-eabi-6-2017-q1-update-linux.tar.bz2 | tar xfj -
        fi
	export PATH=$PATH:`pwd`/gcc-arm-none-eabi-6-2017-q1-update/bin
    fi
    return 0
fi
