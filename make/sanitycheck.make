# Simple checks and warnings - mainly to deal with people using old build commands

ifdef PUCKJS
$(error You now need to build with 'BOARD=PUCKJS make')
endif

ifdef ESPRUINO_1V3
$(error You now need to build with 'BOARD=ESPRUINOBOARD make')
endif

ifdef PICO_1V3
$(error You now need to build with 'BOARD=PICO_R1_3 make')
endif

ifdef ESPRUINOWIFI
$(error You now need to build with 'BOARD=ESPRUINOWIFI make')
endif

ifdef ESP8266
$(error You now need to build with 'BOARD=ESP8266 make')
endif

ifdef ESP32
$(error You now need to build with 'BOARD=ESP32 make')
endif
