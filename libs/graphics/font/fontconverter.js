#!/usr/bin/node
/*
 This file is part of Espruino, a JavaScript interpreter for Microcontrollers

 Copyright (C) 2023 Gordon Williams <gw@pur3.co.uk>

 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.

 ----------------------------------------------------------------------------------------
  Bitmap font creator for Espruino Graphics custom fonts

  Takes input as a PNG font map, PBFF, or bitfontmaker2 JSON

  Outputs in various formats to make a custom font
 ----------------------------------------------------------------------------------------

Requires:

npm install btoa pngjs
git clone https://github.com/espruino/EspruinoWebTools
  (at the same base path as this Espruino Repository)
*/


load = require("../../../../EspruinoWebTools/fontconverter.js").load;


/* load() loads a font. fontInfo should be:
  {
    fn : "font6x8.png", // currently a built-in font
    height : 8, // actual used height of font map
    firstChar : 32,
    maxChars :  256-32
  }

  or:

  {
    fn : "renaissance_28.pbff",
    height : 28, // actual used height of font map
    firstChar : 32,
    yOffset : 4,
    maxChars : 256-32
  }

  or for a font made using https://www.pentacom.jp/pentacom/bitfontmaker2/

  {
    fn : "bitfontmaker2_14px.json",
    height : 14, // actual used height of font map
    firstChar : 32,
    maxChars : 256-32
  }


  Afterwards returns a Font object populated with the args given, and
  a `function getCharPixel(ch,x,y)` which can be used to get the font data
*/

/*var f = load({
fn : "Light20px.png",
bpp : 2,
height : 20, // actual used height of font map
firstChar : 32,
maxChars :  256-32
});
f.fullHeight = false;
f.glyphPadX = 0;*/

/*var f = load({
  fn : "fontx14.png",
  height : 14, // actual used height of font map
  firstChar : 32,
  maxChars : 6*16
});
var f = load({
  fn : "font4x6.png", // currently a built-in font
  height : 6, // actual used height of font map
  firstChar : 32,
  maxChars : 6*16+1
  // fixed width
});
var f = load({
  fn : "font6x8.png", // currently a built-in font
  height : 8, // actual used height of font map
  firstChar : 32,
  maxChars :  256-32
  // fixed width
});
var f = load({
  fn : "renaissance_28.pbff",
  height : 28, // actual used height of font map
  firstChar : 32,
  //yOffset : 4,
});
  var f = load({
    fn : "bitfontmaker_14px.json",
    height : 14, // actual used height of font map
    firstChar : 32,
    maxChars : 256-32
  });*/
var f = load({
  fn : "unifont-15.1.04.png",
  mapWidth : 256, mapHeight : 256,
  mapOffsetX : 32, mapOffsetY : 64,
  height : 16, // actual used height of font map
  //firstChar : 32, maxChars : 6*16
  //firstChar : 0x4E00, maxChars : 0x9FFF - 0x4E00
  //firstChar : 0x8000, maxChars : 0x9FFF - 0x8000
  firstChar : 32, maxChars : 0xD7FF - 32 // all inc korean
});

f.debugChars();
//f.debugPixelsUsed();

//console.log(f.outputJS());

// Write a binary PBF file
require("fs").writeFileSync("font.pbf", Buffer.from(f.getPBF()))

// Write a PBF file as a jswrap_ C file that can be included in the build
// by adding 'WRAPPERSOURCES += libs/graphics/jswrap_font_light20.c' to the BOARD.py file
/*f.getPBFAsC({
  name: "Light20",
  path: __dirname+"/../",
  filename: "jswrap_font_light20"
});*/

//console.log(f.getHeaderFile());
//console.log(f.getJS());
