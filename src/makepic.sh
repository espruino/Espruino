#!/bin/bash
sdcc -mpic16 -p18f8627 --use-non-free -DUSE_FLOATS jspic.c
#sdcc -mpic16 -p18f2620 --use-non-free -DUSE_FLOATS jspic.c
