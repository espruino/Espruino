#!/usr/bin/node
/*
 This file is part of Espruino, a JavaScript interpreter for Microcontrollers

 Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>

 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.

 ----------------------------------------------------------------------------------------
  Bitmap font creator for Graphics custom fonts

  Takes input as a PNG font map, PBFF, or bitfontmaker2 JSON
  
  Outputs as JS to make a custom font
 ----------------------------------------------------------------------------------------
*/

// npm install btoa pngjs


var FIXEDWIDTH = false;
var FIRSTCHAR = 32;
var MAXCHARS = 189; // excludes FIRSTCHAR offset

var FONT = {
  fn : "fontx14.png",
  height : 14, // actual used height of font map
  firstChar : 32,
  maxChars : 6*16
};
var FONT = {
  fn : "font4x6.png", // currently a built-in font
  height : 6, // actual used height of font map
  firstChar : 32,
  maxChars : 6*16+1
  // fixed width
};
var FONT = {
  fn : "font6x8.png", // currently a built-in font
  height : 8, // actual used height of font map
  firstChar : 32,
  maxChars :  256-32
  // fixed width
};

// A font made using https://www.pentacom.jp/pentacom/bitfontmaker2/
/*var FONT = {
  fn : "bitfontmaker2_14px.json",
  height : 14, // actual used height of font map
  firstChar : 32,
  maxChars : 256-32
};*/

// Pebble (Rebble) fonts from https://github.com/pebble-dev/renaissance/tree/master/files
/*var FONT = {
  fn : "renaissance_18_bold.pbff",
  height : 18, // actual used height of font map
  firstChar : 32,
  yOffset : 3,
  maxChars : 256-32
};*/
/*var FONT = {
  fn : "renaissance_28.pbff",
  height : 28, // actual used height of font map
  firstChar : 32,
  yOffset : 4,
  maxChars : 256-32
};*/






var bits = [];
var charWidths = [];

/*
// image width and height, character width and height
var W = 128;
var H = 72;
var CW = 8;
var CH = 12;
var FILENAME = "charset_8x12.raw";

var W = 192;
var H = 24;
var CW = 6;
var CH = 8;
var FILENAME = "charset_6x8.raw";
*/

function loadPNG() {
  var PNG = require("pngjs").PNG;
  var png = PNG.sync.read(require("fs").readFileSync(FONT.fn));

  console.log(`Font map is ${png.width}x${png.height}`);
  FONT.fmWidth = png.width>>4;
  FONT.fmHeight = png.height>>4;
  console.log(`Font map char is ${FONT.fmWidth}x${FONT.fmHeight}`);

  function getPngPixel(x,y) {
    var o = (x + (y*png.width))*4;
    var c = png.data.readInt32LE(o);
    var a = (c>>24)&255;
    var b = (c>>16)&255;;
    var g = (c>>8)&255;
    var r = c&255;
    //console.log(x,y,c.toString(16), a,r,g,b,"=>",(a>128) && ((r+g+b)<384));
    return (a>128) && ((r+g+b)<384);
  }

  FONT.getCharPixel = function(ch,x,y) {
    var chx = ch&15;
    var chy = ch>>4;
    var py = chy*FONT.fmHeight + y;
    if (py>=png.height) return false;
    return getPngPixel(chx*FONT.fmWidth + x, py);
  };
}

function loadJSON() {
  // format used by https://www.pentacom.jp/pentacom/bitfontmaker2/editfont.php import/export
  var font = JSON.parse(require("fs").readFileSync(FONT.fn).toString());
  FONT.fmWidth = 16;
  FONT.fmHeight = 16;
  
  FONT.getCharPixel = function(ch,x,y) {
    if (!font[ch]) return false;
    return ((font[ch][y] >> x) & 1)!=0;
  };  
}

function loadPBFF() {
  // format used by https://github.com/pebble-dev/renaissance/tree/master/files
  FONT.fmWidth = 0;
  FONT.fmHeight = FONT.height;
  var current = {
    idx : 0,
    bmp : []
  };
  var font = [];
  require("fs").readFileSync(FONT.fn).toString().split("\n").forEach(l => {
    if (l.startsWith("version")) {
    } else if (l.startsWith("fallback")) {
    } else if (l.startsWith("line-height")) {
    } else if (l.startsWith("glyph")) {
      current = {};
      current.idx = parseInt(l.trim().split(" ")[1]);
      current.bmp = [];
      font[current.idx] = current;
    } else if (l.trim().startsWith("-")) {
      // font line start/end
      if (l=="-") {
        //console.log(current); // end of glyph
      } else {
        var verticalOffset = parseInt(l.trim().split(" ")[1]);
        while (verticalOffset--) current.bmp.push("");    
      }
    } else if (l.startsWith(" ") || l.startsWith("#")) {
      current.bmp.push(l);    
      if (l.length > FONT.fmWidth) FONT.fmWidth = l.length;  
    } else console.log(`Unknown line '${l}'`);
  });

  
  FONT.getCharPixel = function(ch,x,y) {
    if (!font[ch]) return false;
    return font[ch].bmp[y] && font[ch].bmp[y][x]=='#';
  };  
}

var getCharPixel;

if (FONT.fn && FONT.fn.endsWith("png")) getCharPixel = loadPNG();
else if (FONT.fn && FONT.fn.endsWith("json")) getCharPixel = loadJSON();
else if (FONT.fn && FONT.fn.endsWith("pbff")) getCharPixel = loadPBFF();
else throw new Error("Unknown font type");

var pixelsUsedInRow = new Array(FONT.height);
pixelsUsedInRow.fill(0);

function genChar(ch) {
 // work out widths
 FONT.yOffset = 0|FONT.yOffset;
 var xStart, xEnd;
 if (FIXEDWIDTH) {
   xStart = 0;
   xEnd = FONT.fmWidth-1;
 } else {
   xStart = FONT.fmWidth;
   xEnd = 0;
   for (var x=0;x<FONT.fmWidth;x++) {
     var set = false;
     for (var y=0;y<FONT.height;y++) {
       set |= FONT.getCharPixel(ch,x,y+FONT.yOffset);
     }
     if (set) {
       if (x<xStart) xStart = x;
       xEnd = x;
     }
   } 
   if (xStart>xEnd) {
     xStart=0;
     xEnd = FONT.fmWidth/2; // treat spaces as half-width
   } else if (xEnd<FONT.fmWidth-1)
     xEnd++; // if not full width, add a space after
 }   
 charWidths.push(xEnd+1-xStart);

 var debugText = [];
 for (var y=0;y<FONT.fmHeight;y++) debugText.push("");
 for (var x=xStart;x<=xEnd;x++) {
    for (var y=0;y<FONT.height;y++) {
      var col = FONT.getCharPixel(ch,x,y+FONT.yOffset);
      if (col) pixelsUsedInRow[y]++;
      debugText[y] += col?"#":".";
      bits.push(col ? 1 : 0);
    }
  }
  console.log("charcode ",FIRSTCHAR+charWidths.length-1);
  console.log(debugText.join("\n"));
  console.log();
}

// get an array of bits
for (var i=FONT.firstChar;i<FONT.firstChar+FONT.maxChars;i++) genChar(i);

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

console.log("Pixels used in rows:", pixelsUsedInRow);

// Outputs as JavaScript for a custom font
function outputJS() {
  console.log("var font = atob(\""+require('btoa')(bytes)+"\");");
  console.log("var widths = atob(\""+require('btoa')(widthBytes)+"\");");
  console.log("g.setFontCustom(font, "+FIRSTCHAR+", widths, "+FONT.height+");");
}

// Output
function outputHeaderFile() {
  var PACK_DEFINE = "PACK_5_TO_32";
  var packedChars = 5;
  var packedPixels = 6;

  function genChar(ch) {
    var r = [];
    for (var y=0;y<FONT.fmHeight;y++) {
      var s = "";
      for (var x=0;x<packedPixels;x++) {
        s+= FONT.getCharPixel(ch,x,y) ? "X" : "_";
      }
      r.push(s);
    }
    return r;
  }


  var ch = FONT.firstChar;
  while (ch < FONT.firstChar+FONT.maxChars) {
    var chars = [];
    for (var i=0;i<packedChars;i++) {
      chars.push(genChar(ch));
      ch++;
    }
    for (var cy=0;cy<FONT.fmHeight;cy++) {
      var s = " "+PACK_DEFINE+"( ";
      for (i=0;i<packedChars;i++) {
        if (i>0) s+=" , ";
        s += chars[i][cy];
      }
      s += " ),";
      console.log(s);
    }
    console.log("");
  }
}


outputJS();
//outputHeaderFile();


