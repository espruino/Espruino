/*
 This file is part of Espruino, a JavaScript interpreter for Microcontrollers

 Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>

 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.

 ----------------------------------------------------------------------------------------
  Bitmap font creator for Graphics custom fonts

  This is pretty rough-and-ready, and in order to easily get bitmap data out of an image
  it requires the image to already be in RAW format.

  For this, use ImageMagick as follows: `convert charset_8x12.png -depth 8 gray:charset_8x12.raw`
 ----------------------------------------------------------------------------------------
*/

// npm install btoa
pixels = require("fs").readFileSync("charset_8x12.raw");
// image width and height
var W = 128;
var H = 72;
// character width and height
var CW = 8;
var CH = 12;
/*
pixels = require("fs").readFileSync("charset_6x8.raw");
var W = 192;
var H = 24;
var CW = 6;
var CH = 8;*/


var bits = [];
var charWidths = [];

function genChar(xo,yo) {
 // work out widths
 var xStart = CW;
 var xEnd = 0;
 for (var x=0;x<CW;x++) {
   var set = false;
   for (var y=0;y<CH;y++) {
     var idx = x+xo+((y+yo)*W);
     set |= pixels[idx]<=128;
   }
   if (set) {
     if (x<xStart) xStart = x;
     xEnd = x;
   }
 } 
 if (xStart>xEnd) {
   xStart=0;
   xEnd = CW/2; // treat space as half-width
 } else if (xEnd<CW-1)
   xEnd++; // if not full width, add a space after
 charWidths.push(xEnd+1-xStart);

  for (var x=xStart;x<=xEnd;x++) {
    for (var y=0;y<CH;y++) {
      var idx = x+xo+((y+yo)*W);
      bits.push((pixels[idx]>128) ? 0 : 1);
    }
  }
}


// get an array of bits
var x=0,y=0;
while (y < H) {
  genChar(x,y);
  x += CW;
  if (x>=W) {
    x = 0;
    y += CH;
  } 
}

// compact array
var bytes = "";
for (var i=0;i<bits.length;i+=8) {
  var byte = 0;
  for (var b=0;b<8;b++)
    if (bits[i+b]===1) byte += 1<<(7-b);
  bytes += String.fromCharCode(byte);
}
// convert width array - widthBytes
var widthBytes = "";
for (i in charWidths)
  widthBytes += String.fromCharCode(charWidths[i]);
// widthBytes = charWidths.map(String.fromCharCode).join(""); doesn't work here - too many 0 chars

console.log("var font = atob(\""+require('btoa')(bytes)+"\");");
console.log("var widths = atob(\""+require('btoa')(widthBytes)+"\");");
console.log("g.setFontCustom(font, 32, widths, "+CH+");");


