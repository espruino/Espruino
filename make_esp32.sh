#! /bin/bash
export RELEASE=1
export BOARD=ESP32
export ESP_APP_TEMPLATE_PATH=/media/psf/Home/Desktop/Espruino/esp-idf-template
export ESP_IDF_PATH=/media/psf/Home/Desktop/Espruino/esp-idf
make clean
make
