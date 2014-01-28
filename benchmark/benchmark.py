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
# Run and time a benchmark on an attached Espruino device
# ----------------------------------------------------------------------------------------

import time
import serial
import sys
import json

DUMP_OUTPUT=False

def run_benchmark(device, filename):
        benchmark = open(filename).read();
       
	ser = serial.Serial(
		port=device, # or /dev/ttyAMA0 for serial on the PI
		baudrate=9600,
		parity=serial.PARITY_NONE,
		stopbits=serial.STOPBITS_ONE,
		bytesize=serial.EIGHTBITS,
		xonxoff=0, rtscts=0, dsrdtr=0,
	)
	ser.open()
	ser.isOpen()

        result = ""
        # clean buffer
        while ser.inWaiting() > 0: 
          while ser.inWaiting() > 0: 
            ser.read(1) 
          time.sleep(0.1)


        command = "function __bench() { "+benchmark+" }\n var ___start = getTime();__bench()var ___end = getTime();print('<<'+'<<<'+(___end-___start)+'>>>'+'>>');\n"
        
	#ser.write(command)
        for c in command:
          ser.write(c);
          while ser.inWaiting() > 0:
            c = ser.read(1)
            if DUMP_OUTPUT: 
              sys.stdout.write(c)
              sys.stdout.flush()
            result=result+c

	endtime = time.time()+60 # wait 60 sec
        finished = False
        while time.time() < endtime and not finished:
          while ser.inWaiting() > 0:
            c = ser.read(1)
            if DUMP_OUTPUT: 
              sys.stdout.write(c)
              sys.stdout.flush()
            result=result+c
          finished = "<<<<<" in result and ">>>>>" in result
	ser.close()
        return result[result.find("<<<<<")+5:result.find(">>>>>")];


# Read 1 analog
#print espruino_cmd("print(analogRead(A1))").strip()
# Read 3 analogs into an array
#print espruino_cmd("print([analogRead(A1),analogRead(A2),analogRead(A3)])").strip().split(',')

if len(sys.argv)!=3:
  print "USAGE: benchmark.py /dev/ttyACM0 simple_loop.js"
  exit(1)

print "TIME['"+sys.argv[2]+"'] = "+run_benchmark(sys.argv[1], sys.argv[2])
	
