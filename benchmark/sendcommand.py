#!/usr/bin/python

import time
import serial
import sys
import json

def espruino_cmd(port, command):
	ser = serial.Serial(
		port=port, # or /dev/ttyAMA0 for serial on the PI
		baudrate=9600,
		parity=serial.PARITY_NONE,
		stopbits=serial.STOPBITS_ONE,
		bytesize=serial.EIGHTBITS,
		xonxoff=0, rtscts=0, dsrdtr=0,
	)
	ser.open()
	ser.isOpen()
        result = ""
        ser.write("\03") # ensure we break out of anything we were doing
	for c in command : 
		while ser.inWaiting() > 0:
			result=result+ser.read(1)
		ser.write(c)
        ser.write("\n")
	endtime = time.time()+0.2 # wait 0.2 sec
	while time.time() < endtime:
		while ser.inWaiting() > 0:
			result=result+ser.read(1)
	ser.close()
        return result


if len(sys.argv)!=3:
  print "USAGE: sendcommand.py /dev/ttyACM0 \"print('hello')\""
  exit(1)

PORT=sys.argv[1]
COMMAND=sys.argv[2]

print espruino_cmd(PORT, COMMAND).strip()
	
