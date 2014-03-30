/*
 This file is part of Espruino, a JavaScript interpreter for Microcontrollers

 Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>

 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.

 ----------------------------------------------------------------------------------------
  Bitmap font header file creator
 ----------------------------------------------------------------------------------------
*/

// convert charset.png -depth 8 gray:charset.raw
pixels = require("fs").readFileSync("charset.raw");
var W=128;
var H=18;
var CW = 4;
var CWX = 3; // 1 of spaces
var CH = 6;
var packedChars = 5;

function genChar(xo,yo) {
  var r = [];
  for (var y=0;y<CH;y++) {
    var idx = xo+((y+yo)*W);
    var s = "";
    for (var x=0;x<CWX;x++) {
      s+= (pixels[idx++]>128) ? "_" : "X";
    }
    r.push(s);
  }
  return r;
}


var x=CW,y=0;

while (y < H) {
  var chars = [];
  for (var i=0;i<packedChars;i++) {
    chars.push(genChar(x,y));
    x += CW;
    if (x>=W) {
      x = 0;
      y += CH;
    } 
  }
  for (var cy=0;cy<CH;cy++) {
    var s = " PACK_5_TO_16( ";
    for (i=0;i<packedChars;i++) {
      if (i>0) s+=" , ";
      s += chars[i][cy];      
    }
    s += " ),";
    console.log(s);
  }
  console.log("");
}


