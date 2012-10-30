#!/bin/bash
cd `dirname $0`
cd ..

awk '/\[TODO\]/{s=x}{s=s$0"\n"}/\[\/TODO\]/{print s}' src/jsutils.h  | tail -n +2 | head -n -2
