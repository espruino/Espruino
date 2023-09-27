#!/usr/bin/env python
# Checks to see if an Elf file will fit in memory before the
# area reserved for Storage (saved code). This is mainly used
# for nRF5x parts (which don't generate binaries and which
# put Storage *after* code)

import subprocess;
import re;
import json;
import sys;
import os;
import importlib;

scriptdir = os.path.dirname(os.path.realpath(__file__))
basedir = scriptdir+"/../"
sys.path.append(basedir+"scripts");
sys.path.append(basedir+"boards");

if len(sys.argv)!=3:
  print("USAGE:")
  print("scripts/check_elf_size.py BOARDNAME board.elf")
  exit(1)

BOARD=sys.argv[1]
ELF=sys.argv[2]

print("Testing "+ELF+" for "+BOARD)
# import the board def
board = importlib.import_module(BOARD)
# Call the included board_specific file - it sets up 'pins' and 'fill_gaps'
storageStart = board.chip['saved_code']['address']
storageEnd = storageStart + board.chip['saved_code']['page_size'] * board.chip['saved_code']['pages']
print("STORAGE: "+hex(storageStart)+" -> "+hex(storageEnd));
areaName = "Storage"
areaStart = storageStart
areaEnd = storageEnd

text = subprocess.check_output('arm-none-eabi-objdump -h '+ELF+' | grep "\\.text"', shell=True).strip().split()
codeSize = int(text[2], 16)
codeStart = int(text[4], 16)
codeEnd = codeSize + codeStart

# nRF52 builds have this extra data dumped on the end - need to check this doesn't overlap too!
fsdata = subprocess.check_output('arm-none-eabi-objdump -h '+ELF+' | grep "\\.fs_data" || true', shell=True).strip().split()
if len(fsdata):
  fsSize = int(fsdata[2], 16)
  fsStart = int(fsdata[4], 16)
  fsEnd = fsStart + fsSize
  if (fsEnd > codeEnd): 
    print("FS DATA: "+hex(fsStart)+" -> "+hex(fsEnd)+" ("+str(fsSize)+" bytes)");
    codeEnd = fsEnd

if storageStart == 0x60000000: # it's the memory-mapped external flash
  if board.chip['part']=="NRF52832":
    areaStart = 118 * 4096 # Bootloader takes pages 120-127, FS takes 118-119
    areaEnd = 120 * 4026
    areaName = "FS"
    print("FLASH_AVAILABLE: "+hex(areaStart));    

print("CODE: "+hex(codeStart)+" -> "+hex(codeEnd)+" ("+str(codeSize)+" bytes)");

if codeEnd<areaStart and codeStart<areaStart:
  print("Code area Fits before "+areaName+" Area ("+str(areaStart-codeEnd)+"b free)")
elif codeEnd>areaEnd and codeStart>areaEnd:
  print("Code area Fits after "+areaName+" Area")
else:
  print("==========================")
  print(" CODE AND STORAGE OVERLAP")
  print("   by "+ str(codeEnd-areaStart) + " bytes")
  print("==========================")
  exit(1)
