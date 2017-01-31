#!/bin/bash

cd `dirname $0`
cd ..

find . | grep -f scripts/count_lines.include | grep -v -f scripts/count_lines.exclude

echo "Total lines"
find . | grep -f scripts/count_lines.include | grep -v -f scripts/count_lines.exclude | xargs cat | wc -l

#Can also count by lines written
#find . | grep -f scripts/count_lines.include | grep -v -f scripts/count_lines.exclude | xargs -I{} git blame {} | sed -ne "s/[^(]*(\([A-Za-z0-9]*\).*/\1/p" > blames
#sort blames | uniq -c | sort -n

