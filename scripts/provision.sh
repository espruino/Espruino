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
  echo "  source scripts/provision.sh {BOARD}"
  echo "  source scripts/provision.sh ALL"
  return 1
fi

# set the current board
BOARDNAME=$1

if [ "$BOARDNAME" = "ALL" ]; then
  echo "Installing dev tools for all boards"
  PROVISION_ESP32=1
  PROVISION_ESP8266=1
  PROVISION_LINUX=1
  PROVISION_NRF52=1
  PROVISION_NRF51=1
  PROVISION_NRF_SDK15=1
  PROVISION_STM32F1=1
  PROVISION_STM32F4=1
  PROVISION_STM32L4=1 
  PROVISION_RASPBERRYPI=1 
else
  FAMILY=`scripts/get_board_info.py $BOARDNAME 'board.chip["family"]'`
  if [ "$FAMILY" = "" ]; then
    echo "UNKNOWN BOARD ($BOARDNAME)"
    return 1
  fi  
  export PROVISION_$FAMILY=1
  export PROVISION_$BOARDNAME=1
  if python scripts/get_makefile_decls.py $BOARDNAME | grep NRF_SDK15; then
    PROVISION_NRF_SDK15=1
  fi
fi
echo Provision BOARDNAME = $BOARDNAME
echo Provision FAMILY = $FAMILY

if [ "$PROVISION_ESP32" = "1" ]; then
    echo ===== ESP32
    # needed for esptool for merging binaries
    if pip --version 2>/dev/null; then 
      echo python/pip installed
    else
      echo Installing python/pip pyserial
      sudo DEBIAN_FRONTEND=noninteractive apt-get install -qq -y python python-pip
    fi
    if pip list 2>/dev/null | grep pyserial >/dev/null; then 
      echo pyserial installed; 
    else 
      echo Installing pyserial
      sudo pip -q install pyserial
    fi    
    # SDK
    if [ ! -d "app" ]; then
        echo installing app folder
        curl -Ls https://github.com/espruino/EspruinoBuildTools/raw/master/esp32/deploy/app.tgz | tar xfz - --no-same-owner
        #curl -Ls https://github.com/espruino/EspruinoBuildTools/raw/ESP32-V3.1/esp32/deploy/app.tgz | tar xfz - --no-same-owner
    fi
    if [ ! -d "esp-idf" ]; then
        echo installing esp-idf folder
        curl -Ls https://github.com/espruino/EspruinoBuildTools/raw/master/esp32/deploy/esp-idf.tgz | tar xfz - --no-same-owner
        #curl -Ls https://github.com/espruino/EspruinoBuildTools/raw/ESP32-V3.1/esp32/deploy/esp-idf.tgz | tar xfz - --no-same-owner
    fi
    if ! type xtensa-esp32-elf-gcc 2> /dev/null > /dev/null; then
        echo installing xtensa-esp32-elf-gcc
        if [ ! -d "xtensa-esp32-elf" ]; then
           #curl -Ls https://dl.espressif.com/dl/xtensa-esp32-elf-linux64-1.22.0-80-g6c4433a-5.2.0.tar.gz | tar xfz - --no-same-owner
           curl -Ls https://github.com/espruino/EspruinoBuildTools/raw/master/esp32/xtensa-esp32-elf-linux64-1.22.0-80-g6c4433a-5.2.0.tar.gz | tar xfz - --no-same-owner
        else
           echo "Folder found"
        fi
    fi
    export ESP_IDF_PATH=`pwd`/esp-idf
    export ESP_APP_TEMPLATE_PATH=`pwd`/app
    export PATH=$PATH:`pwd`/xtensa-esp32-elf/bin/
    echo GCC is $(which xtensa-esp32-elf-gcc)
fi
#--------------------------------------------------------------------------------
if [ "$PROVISION_ESP8266" = "1" ]; then
    echo ===== ESP8266
    if [ ! -d "ESP8266_NONOS_SDK-2.2.1" ]; then
        echo ESP8266_NONOS_SDK-2.2.1
        curl -Ls https://github.com/espruino/EspruinoBuildTools/raw/master/esp8266/ESP8266_NONOS_SDK-2.2.1.tar.gz | tar xfz - --no-same-owner
    fi
    if ! type xtensa-lx106-elf-gcc 2> /dev/null > /dev/null; then
        echo installing xtensa-lx106-elf-gcc
        if [ ! -d "xtensa-lx106-elf" ]; then
            curl -Ls https://github.com/espruino/EspruinoBuildTools/raw/master/esp8266/xtensa-lx106-elf-20160330.tgx | tar Jxf - --no-same-owner
        else
            echo "Folder found"
        fi

    fi
    export ESP8266_SDK_ROOT=`pwd`/ESP8266_NONOS_SDK-2.2.1
    export PATH=$PATH:`pwd`/xtensa-lx106-elf/bin/
    echo GCC is $(which xtensa-lx106-elf-gcc)
fi
#--------------------------------------------------------------------------------
if [ "$PROVISION_LINUX" = "1" ]; then
    echo ===== LINUX
fi
#--------------------------------------------------------------------------------
if [ "$PROVISION_RASPBERRYPI" = "1" ]; then
    echo ===== RASPBERRYPI
    if [ ! -d "targetlibs/raspberrypi" ]; then
        echo Installing Raspberry pi tools
        mkdir targetlibs/raspberrypi        
        cd targetlibs/raspberrypi
        git clone --depth=1 https://github.com/raspberrypi/tools
        # wiringpi?
        cd ../..
    fi    
fi
#--------------------------------------------------------------------------------
if [ "$PROVISION_NRF52" = "1" ]; then
    echo ===== NRF52
    if ! type pip 2> /dev/null > /dev/null; then
      echo Installing python and pip
      sudo DEBIAN_FRONTEND=noninteractive apt-get install -qq -y python python-pip
    fi
    if ! type nrfutil 2> /dev/null > /dev/null; then
      echo Installing nrfutil
      sudo pip install --ignore-installed nrfutil
      # --ignore-installed is used because pip 10 fails because PyYAML was already installed by the system
      # -q can be used to silence the above
    fi
    ARM=1

fi
#--------------------------------------------------------------------------------
if [ "$PROVISION_NRF51" = "1" ]; then
    ARM=1
fi
if [ "$PROVISION_NRF_SDK15" = "1" ]; then
    if [ ! -d "targetlibs/nrf5x_15/components" ]; then
        echo Installing NRF SDK 15.0 to targetlibs/nrf5x_15/components
        # curl https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v15.x.x/nRF5_SDK_15.0.0_a53641a.zip -o nRF5_SDK_15.0.0_a53641a.zip
        curl -Ls https://github.com/espruino/EspruinoBuildTools/raw/master/nrf52/nRF5_SDK_15.0.0_a53641a_no_docs_unix.zip -o nRF5_SDK_15.0.0_a53641a.zip
        # This is nRF5_SDK_15.0.0_a53641a.zip without the docs/examples folder, and with line endings converted to unix (for patch)
        unzip -q -o nRF5_SDK_15.0.0_a53641a.zip
        mv nRF5_SDK_15.0.0_a53641a/* targetlibs/nrf5x_15
        rm -rf nRF5_SDK_15.0.0_a53641a.zip nRF5_SDK_15.0.0_a53641a
        echo ======================================================
        echo "FIXME - SDK15 NFC patches don't apply cleanly"
        echo ======================================================
        cat targetlibs/nrf5x_15/patches/* | unix2dos | patch -p1
    fi
fi
#--------------------------------------------------------------------------------
if [ "$PROVISION_STM32F1" = "1" ]; then
    ARM=1
fi
#--------------------------------------------------------------------------------
if [ "$PROVISION_STM32F3" = "1" ]; then
    ARM=1
fi
#--------------------------------------------------------------------------------
if [ "$PROVISION_STM32F4" = "1" ]; then
    ARM=1
fi
#--------------------------------------------------------------------------------
if [ "$PROVISION_STM32L4" = "1" ]; then
    ARM=1
fi
#--------------------------------------------------------------------------------
if [ "$PROVISION_EFM32GG" = "1" ]; then
    ARM=1
fi
#--------------------------------------------------------------------------------
if [ "$PROVISION_SAMD" = "1" ]; then
    ARM=1
fi
#--------------------------------------------------------------------------------


if [ "$ARM" = "1" ]; then
    # defaulting to ARM
    echo ===== ARM
    if type arm-none-eabi-gcc 2> /dev/null > /dev/null; then
        echo arm-none-eabi-gcc installed
    else
        echo installing gcc-arm-embedded
        #sudo add-apt-repository -y ppa:team-gcc-arm-embedded/ppa
        #sudo apt-get update
        #sudo DEBIAN_FRONTEND=noninteractive apt-get --force-yes --yes install libsdl1.2-dev gcc-arm-embedded
        # Unpack - newer, and much faster
        if [ ! -d "gcc-arm-none-eabi-8-2018-q4-major" ]; then
          curl -Ls https://github.com/espruino/EspruinoBuildTools/raw/master/arm/gcc-arm-none-eabi-8-2018-q4-major-linux.tar.bz2 | tar xfj - --no-same-owner
        else
            echo "Folder found"
        fi
	export PATH=$PATH:`pwd`/gcc-arm-none-eabi-8-2018-q4-major/bin
    fi
fi
