#!/usr/bin/python

# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# ----------------------------------------------------------------------------------------
# Utility function - return the actual bytes which come from the serial port
# ----------------------------------------------------------------------------------------

import time
import serial
import sys
import json


ser = serial.Serial(
	port='/dev/ttyUSB0', # or /dev/ttyAMA0 for serial on the PI
	baudrate=9600,
	parity=serial.PARITY_NONE,
	stopbits=serial.STOPBITS_ONE,
	bytesize=serial.EIGHTBITS,
	xonxoff=0, rtscts=0, dsrdtr=0,
)
ser.open()
ser.isOpen()
endtime = time.time()+0.2 # wait 0.2 sec
while True:
	while ser.inWaiting() > 0:
		print(str(ord(ser.read(1))))
ser.close()



