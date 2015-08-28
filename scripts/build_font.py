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
# Bitmap font creator - not actually used yet though
# ----------------------------------------------------------------------------------------

import Image
import ImageFont, ImageDraw
import textwrap

font = ImageFont.truetype("/usr/share/fonts/truetype/helvetica/Helvetica.ttf", 10)

fontbitmap = []
fontoffset = []


for charNum in range(0,127):
  char = chr(charNum)
  image = Image.new('RGB', (20, 20))
  draw = ImageDraw.Draw(image)
  draw.text((0,0), char, font=font)
  width = font.getsize(char)[0]
  pixels = image.load() 
  print(width)
  bits = 0
  for x in range(0,width):
    bits = 0
    for y in range(0,8):
       bit = pixels[x,y+1][0] > 127
       bits = (bits<<1) | bit
    fontbitmap.append(str(bits))
  # insert a space if we need one
  if bits!=0: fontbitmap.append(0)
  fontoffset.append(str(len(fontbitmap)))

for x in fontbitmap:
 s = "|"
 bits = int(x)
 for y in range(0,8):
   if bits&1:
     s = s+"#"
   else:
     s = s+" "
   bits = bits>>1
 print(s+"|")


print("unsigned char fontBitmap["+str(len(fontbitmap))+"] = {")
print("\n".join(textwrap.wrap(",".join(fontbitmap))))
print("};")
print("unsigned short fontOffsets["+str(len(fontoffset))+"] = {")
print("\n".join(textwrap.wrap(",".join(fontoffset))))
print("};")

exit(0)


