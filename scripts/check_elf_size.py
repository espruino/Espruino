#!/usr/bin/python
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
print("STORAGE: "+str(storageStart)+" -> "+str(storageEnd));

text = subprocess.check_output('arm-none-eabi-objdump -h '+ELF+' | grep "\\.text"', shell=True).strip().split()
codeSize = int(text[2], 16)
codeStart = int(text[3], 16)
codeEnd = codeSize + codeStart

print("CODE: "+str(codeStart)+" -> "+str(codeEnd));

if codeEnd<storageStart and codeStart<storageStart:
  print("Code area Fits before Storage Area")
elif codeEnd>storageEnd and codeStart>storageEnd:
  print("Code area Fits after Storage Area")
else:
  print("CODE AND STORAGE OVERLAP")
  exit(1)
