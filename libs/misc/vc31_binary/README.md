Proprietary VC31 HRM algorithm
===============================

In this directory are the object files and header for Vcare's heart rate algorithm.

These were created by VCare (the VC31 manufacturer) and are not part of the MPLv2 license that covers the rest of the Espruino interpreter.


Usage
-----

Instead of the code below in the `BOARD.py` file that enables the open source heart rate algorithm:

```
'SOURCES += libs/misc/heartrate.c',
```

you'll need:

```
'SOURCES += libs/misc/heartrate_vc31_binary.c',
'DEFINES += -DHEARTRATE_VC31_BINARY=1',
'PRECOMPILED_OBJS += libs/misc/vc31_binary/algo.o libs/misc/vc31_binary/modle5_10.o libs/misc/vc31_binary/modle5_11.o libs/misc/vc31_binary/modle5_12.o libs/misc/vc31_binary/modle5_13.o libs/misc/vc31_binary/modle5_14.o libs/misc/vc31_binary/modle5_15.o libs/misc/vc31_binary/modle5_16.o libs/misc/vc31_binary/modle5_17.o libs/misc/vc31_binary/modle5_18.o libs/misc/vc31_binary/modle5_1.o libs/misc/vc31_binary/modle5_2.o libs/misc/vc31_binary/modle5_3.o libs/misc/vc31_binary/modle5_4.o libs/misc/vc31_binary/modle5_5.o libs/misc/vc31_binary/modle5_6.o libs/misc/vc31_binary/modle5_7.o libs/misc/vc31_binary/modle5_8.o libs/misc/vc31_binary/modle5_9.o',
```