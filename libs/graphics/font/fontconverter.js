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
*/

// npm install btoa pngjs

function bitsToBytes(bits, bpp) {
  var bytes = [];
  if (bpp==1) {
    for (var i=0;i<bits.length;i+=8) {
      var byte = 0;
      for (var b=0;b<8;b++)
        byte |= (bits[i+b]) << (7-b);
      bytes.push(byte);
    }
  } else if (bpp==2) {
    for (var i=0;i<bits.length;i+=4) {
      var byte = 0;
      for (var b=0;b<4;b++)
        byte |= bits[i+b] << (6-b*2);
      bytes.push(byte);
    }
  } else throw "unknown bpp";
  return bytes;
}

/*
{
  fn
  mapWidth,mapHeight
  mapOffsetX, mapOffsetY
  height
}
*/
function Font(info) {
  this.name = info.name;
  this.id = this.name ? this.name.replace(/[^A-Za-z0-9]/g,"") : "Unknown";
  this.fn = info.fn;
  this.height = info.height;
  this.bpp = info.bpp||1;
  this.firstChar = (info.firstChar!==undefined) ? info.firstChar : 32;
  this.maxChars = info.maxChars || (256-this.firstChar);
  this.lastChar = this.firstChar + this.maxChars - 1;
  this.fixedWidth = !!info.fixedWidth;
  this.fullHeight = !!info.fullHeight; // output fonts at the full height available
  this.glyphPadX = 1; // padding at the end of glyphs (needed for old-style JS fonts)
  this.glyphs = [];
  // set up later:
  // fmWidth  - max width of any character (we scan this area when writing fonts, even if we crop down later)
  // fmHeight - max height of any character (we scan this area when writing fonts, even if we crop down later)
  this.mapWidth = info.mapWidth||16; // for loadPNG
  this.mapHeight = info.mapHeight||16;
  this.mapOffsetX = 0|info.mapOffsetX;
  this.mapOffsetY = 0|info.mapOffsetY;
}

function FontGlyph(font, ch, getPixel) {
  this.font = font;
  this.ch = ch;
  this.getPixel = getPixel;
  // set up later:
  //
}

// Populate the `glyphs` array with a range of glyphs
Font.prototype.generateGlyphs = function(getCharPixel) {
  for (let ch=this.firstChar; ch<=this.lastChar; ch++)
    this.glyphs[ch] = this.getGlyph(ch, (x,y) => getCharPixel(ch,x,y));
};

// Append the bits to define this glyph to the array 'bits'
FontGlyph.prototype.appendBits = function(bits, info) {
  // glyphVertical : are glyphs scanned out vertically or horizontally?
  if (info.glyphVertical) {
    for (var x=this.xStart;x<=this.xEnd;x++) {
      for (var y=this.yStart;y<=this.yEnd;y++) {
        bits.push(this.getPixel(x,y));
      }
    }
  } else {
    for (var y=this.yStart;y<=this.yEnd;y++) {
      for (var x=this.xStart;x<=this.xEnd;x++) {
        bits.push(this.getPixel(x,y));
      }
    }
  }
 }

 FontGlyph.prototype.getImageData = function() {
   var bpp = this.font.bpp;
   var img = new ImageData(this.xEnd+1, this.yEnd+1);
   for (var y=0;y<=this.yEnd;y++)
     for (var x=0;x<=this.xEnd;x++) {
      var n = (x + y*img.width)*4;
      var c = this.getPixel(x,y);
      var prevCol = 255 - ((bpp==1) ? c*255 : (c << (8-bpp)));
      img.data[n] = img.data[n+1] = img.data[n+2] = prevCol;
      if (x>=this.xStart && y>=this.yStart) {
        img.data[n] = 128;
        img.data[n+3] = 255;
      }
     }
   return img;
 };

 FontGlyph.prototype.debug = function() {
  var map = ".#";//"░█";
  if (this.font.bpp==2) map = "░▒▓█";
  var debugText = [];
  for (var y=0;y<this.font.fmHeight;y++) debugText.push("");
  for (var x=this.xStart;x<=this.xEnd;x++) {
    for (var y=0;y<this.font.fmHeight;y++) {
      var col = this.getPixel(x,y);
      debugText[y] += (y>=this.yStart && y<=this.yEnd) ? map[col] : ".";
    }
  }
  console.log("charcode ", this.ch);
  console.log(debugText.join("\n"));
};

 Font.prototype.getGlyph = function(ch, getPixel) {
  // work out widths
  var glyph = new FontGlyph(this, ch, getPixel);
  var xStart, xEnd;
  var yStart = this.fmHeight, yEnd = 0;

  if (this.fixedWidth) {
    xStart = 0;
    xEnd = this.fmWidth-1;
  } else {
    xStart = this.fmWidth;
    xEnd = 0;
    for (var x=0;x<this.fmWidth;x++) {
      for (var y=0;y<this.fmHeight;y++) {
        var set = glyph.getPixel(x,y);
        if (set) {
          // check X max value
          if (x<xStart) xStart = x;
          xEnd = x;
          // check Y max/min
          if (y<yStart) yStart = y;
          if (y>yEnd) yEnd = y;
        }
      }
    }
    if (xStart>xEnd) {
      xStart=0;
      xEnd = this.fmWidth >> 1; // treat spaces as half-width
    } else if (xEnd<this.fmWidth-1)
      xEnd += this.glyphPadX; // if not full width, add a space after
  }
  glyph.width = xEnd+1-xStart;
  glyph.xStart = xStart;
  glyph.xEnd = xEnd;
  glyph.advance = glyph.width;
  if (!this.glyphPadX) glyph.advance++; // hack - add once space of padding

  if (this.fullHeight) {
    yStart = 0;
    yEnd = this.fmHeight-1;
  }
  if (yStart>=yEnd) {
    yStart = 1;
    yEnd = 0;
  }
  glyph.yStart = yStart;
  glyph.yEnd = yEnd;
  glyph.height = yEnd+1-yStart;

  return glyph;
};

// Load a 16x16 charmap file (or mapWidth x mapHeight)
function loadPNG(fontInfo) {
  fontInfo = new Font(fontInfo);
  var PNG = require("pngjs").PNG;
  var png = PNG.sync.read(require("fs").readFileSync(fontInfo.fn));

  console.log(`Font map is ${png.width}x${png.height}`);
  fontInfo.fmWidth = Math.floor((png.width - fontInfo.mapOffsetX) / fontInfo.mapWidth);
  fontInfo.fmHeight = Math.floor((png.height - fontInfo.mapOffsetY) / fontInfo.mapHeight);
  console.log(`Font map char is ${fontInfo.fmWidth}x${fontInfo.fmHeight}`);

  function getPngPixel(x,y) {
    var o = (x + (y*png.width))*4;
    var c = png.data.readInt32LE(o);
    var a = (c>>24)&255;
    var b = (c>>16)&255;;
    var g = (c>>8)&255;
    var r = c&255;
    if (a<128) return 0; // no alpha
    var avr = (r+g+b)/3;
    if (fontInfo.bpp==1) return 1-(avr>>7);
    if (fontInfo.bpp==2) return 3-(avr>>6);
    throw new Error("Unknown bpp");
    //console.log(x,y,c.toString(16), a,r,g,b,"=>",(a>128) && ((r+g+b)<384));
  }

  fontInfo.generateGlyphs(function(ch,x,y) {
    var chy = Math.floor(ch/fontInfo.mapWidth);
    var chx = ch - chy*fontInfo.mapWidth;
    var py = chy*fontInfo.fmHeight + y;
    if (py>=png.height) return false;
    return getPngPixel(fontInfo.mapOffsetX + chx*fontInfo.fmWidth + x, fontInfo.mapOffsetY + py);
  });
  return fontInfo;
}

function loadJSON(fontInfo) {
  fontInfo = new Font(fontInfo);
  // format used by https://www.pentacom.jp/pentacom/bitfontmaker2/editfont.php import/export
  var font = JSON.parse(require("fs").readFileSync(fontInfo.fn).toString());
  fontInfo.fmWidth = 16;
  fontInfo.fmHeight = 16;

  fontInfo.generateGlyphs(function(ch,x,y) {
    if (!font[ch]) return 0;
    return (((font[ch][y] >> x) & 1)!=0) ? 1 : 0;
  });
  return fontInfo;
}

function loadPBFF(fontInfo) {
  // format used by https://github.com/pebble-dev/renaissance/tree/master/files
  fontInfo = new Font(fontInfo);
  fontInfo.fmWidth = 0;
  fontInfo.fmHeight = fontInfo.height;
  fontInfo.glyphPadX = 0;
  fontInfo.fullHeight = false;
  var current = {
    idx : 0,
    bmp : []
  };
  var font = [];
  require("fs").readFileSync(fontInfo.fn).toString().split("\n").forEach(l => {
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
    } else if (l.startsWith(" ") || l.startsWith("#") || l=="") {
      current.bmp.push(l);
      if (l.length > fontInfo.fmWidth) fontInfo.fmWidth = l.length;
      if (current.bmp.length > fontInfo.fmHeight) {
        console.log(current.idx+" bump height to "+current.bmp.length);
        fontInfo.fmHeight = current.bmp.length;
      }
    } else if (l!="") console.log(`Unknown line '${l}'`);
  });

  fontInfo.generateGlyphs(function(ch,x,y) {
    if (!font[ch]) return 0;
    return (font[ch].bmp[y] && font[ch].bmp[y][x]=='#') ? 1 : 0;
  });
  return fontInfo;
}

function load(fontInfo) {
  if (fontInfo.fn && fontInfo.fn.endsWith("png")) return loadPNG(fontInfo);
  else if (fontInfo.fn && fontInfo.fn.endsWith("json")) return loadJSON(fontInfo);
  else if (fontInfo.fn && fontInfo.fn.endsWith("pbff")) return loadPBFF(fontInfo);
  else throw new Error("Unknown font type");
}



Font.prototype.debugPixelsUsed = function() {
  var pixelsUsedInRow = new Array(this.height);
  pixelsUsedInRow.fill(0);
  Object.keys(this.glyphs).forEach(ch => {
    var glyph = this.glyphs[ch];
    for (var x=glyph.xStart;x<=glyph.xEnd;x++) {
      for (var y=0;y<this.height;y++) {
        var col = glyph.getPixel(x,y);
        if (col) pixelsUsedInRow[y]++;
      }
    }
  });
  console.log("Pixels used in rows:", JSON.stringify(pixelsUsedInRow,null,2));
};

Font.prototype.debugChars = function() {
  Object.keys(this.glyphs).forEach(ch => {
    this.glyphs[ch].debug();
    console.log();
  });
};

// Outputs as JavaScript for a custom font
Font.prototype.getJS = function(options) {
  // options.compressed
  options = options||{};
  this.glyphPadX = 1;
  var charMin = Object.keys(this.glyphs)[0];
  // stats
  var minY = this.height;
  var maxY = 0;
  // get an array of bits
  var bits = [];
  var charGlyphs = [];
  Object.keys(this.glyphs).forEach(ch => {
    var glyph = this.glyphs[ch];
    if (glyph.yEnd > maxY) maxY = glyph.yEnd;
    if (glyph.yStart < minY) minY = glyph.yStart;
  });
  Object.keys(this.glyphs).forEach(ch => {
    var glyph = this.glyphs[ch];
    glyph.xStart = 0; // all glyphs have to start at 0 now
    glyph.yStart = 0;
    glyph.xEnd = glyph.width-1;
    glyph.yEnd = this.height-1;
    glyph.height = this.height;
    glyph.appendBits(bits, {glyphVertical:true});
  });
  // compact array
  var fontData = bitsToBytes(bits, this.bpp);
  // convert width array - widthBytes
  var fontWidths = [];
  Object.keys(this.glyphs).forEach(ch => {
    fontWidths[ch] = this.glyphs[ch].width;
  });
  fontWidths = fontWidths.slice(charMin); // don't include chars before we're outputting
  var fixedWidth = fontWidths.every(w=>w==fontWidths[0]);

  var encodedFont;
  if (options.compressed) {
    const fontArray = new Uint8Array(fontData);
    const compressedFont = String.fromCharCode.apply(null, heatshrink.compress(fontArray));
    encodedFont =
      "E.toString(require('heatshrink').decompress(atob('" +
      btoa(compressedFont) +
      "')))";
  } else {
    encodedFont = "atob('" + btoa(String.fromCharCode.apply(null, fontData)) + "')";
  }

/*  return `
var font = atob("${require('btoa')(bytes)}");
var widths = atob("${require('btoa')(widthBytes)}");
g.setFontCustom(font, ${this.firstChar}, widths, ${this.height} | ${this.bpp<<16});
`;*/

  return `Graphics.prototype.setFont${this.id} = function() {
  // Actual height ${maxY+1-minY} (${maxY} - ${minY})
  // ${this.bpp} BPP
  return this.setFontCustom(
    ${encodedFont},
    ${charMin},
    ${fixedWidth?fontWidths[0]:`atob("${btoa(String.fromCharCode.apply(null,fontWidths))}")`},
    ${this.height}|${this.bpp<<16}
  );
}\n`;
}

// Output to a C header file (only works for 6px wide)
Font.prototype.getHeaderFile = function() {
  var PACK_DEFINE = "PACK_5_TO_32";
  var packedChars = 5;
  var packedPixels = 6;

  function genChar(font, glyph) {
    var r = [];
    for (var y=0;y<font.fmHeight;y++) {
      var s = "";
      for (var x=0;x<packedPixels;x++) {
        s+= glyph.getPixel(x + glyph.xStart,y) ? "X" : "_";
      }
      r.push(s);
    }
    return r;
  }

  var header = "";
  var ch = this.firstChar;
  while (ch <= this.lastChar) {
    var chars = [];
    for (var i=0;i<packedChars;i++) {
      var glyph = this.glyphs[ch];
      if (glyph===undefined)
        glyph = { getPixel : () => false };
      chars.push(genChar(this, glyph));
      ch++;
    }
    for (var cy=0;cy<this.fmHeight;cy++) {
      var s = " "+PACK_DEFINE+"( ";
      for (i=0;i<packedChars;i++) {
        if (i>0) s+=" , ";
        s += chars[i][cy];
      }
      s += " ),";
      header += s+"\n";
    }
    header += "\n";
  }
  return header;
}

// Output as a PBF file
Font.prototype.getPBF = function() {
  // https://github.com/pebble-dev/wiki/wiki/Firmware-Font-Format
  // setup to ensure we're not writing entire glyphs
  this.glyphPadX = 0;
  this.fullHeight = false; // TODO: too late?
  // now go through all glyphs
  var glyphs = [];
  var hashtableSize = ((this.lastChar-this.firstChar)>1000) ? 255 : 64;
  var hashes = [];
  for (var i=0;i<hashtableSize;i++)
    hashes[i] = [];
  var dataOffset = 0;
  var allOffsetsFitIn16Bits = true;
  Object.keys(this.glyphs).forEach(ch => {
    var bits = [];
    var glyph = this.glyphs[ch];
    glyph.appendBits(bits, {glyphVertical:false});
    glyph.bits = bits;
    glyph.bpp = this.bpp;
    // check if this glyph is just 1bpp - if so convert it
    if (glyph.bpp==2) {
      if (!glyph.bits.some(b => (b==1) || (b==2))) {
        //console.log(String.fromCharCode(glyph.ch)+" is 1bpp");
        glyph.bpp=1;
        glyph.bits = glyph.bits.map(b => b>>1);
      }
    }
    glyphs.push(glyph);
    glyph.hash = ch%hashtableSize;
    glyph.dataOffset = dataOffset;
    if (dataOffset > 65535)
      allOffsetsFitIn16Bits = false;
    dataOffset += 5 + ((glyph.bits.length*glyph.bpp + 7)>>3); // supposedly we should be 4 byte aligned, but there seems no reason?
    hashes[glyph.hash].push(glyph);
  });

  var useExtendedHashTableOffset = (6 * glyphs.length) > 65535;
  var use16BitOffsets = allOffsetsFitIn16Bits;
  var version = 3;//useExtendedHashTableOffset ? 3 : 2;
  if (version==2 && allOffsetsFitIn16Bits) throw new Error("16 bit offsets not supported in PBFv2");
  if (version==2 && useExtendedHashTableOffset) throw new Error("24 bit hashtable offsets not supported in PBFv2");
  console.log("Using PBF version "+version);
  console.log("  16 Bit Offsets = "+use16BitOffsets);
  console.log("  24 Bit HashTable = "+useExtendedHashTableOffset);

  var pbfOffsetTableEntrySize = use16BitOffsets ? 4 : 6;

  var pbfHeader = new DataView(new ArrayBuffer((version>=3) ? 10 : 8));
  pbfHeader.setUint8(0, version); // version
  pbfHeader.setUint8(1, this.height); // height
  pbfHeader.setUint16(2, glyphs.length, true/*LE*/); // glyph count
  pbfHeader.setUint16(4, 0, true/*LE*/); // wildcard codepoint
  pbfHeader.setUint8(6, hashtableSize); // hashtable Size
  pbfHeader.setUint8(7, 2); // codepoint size
  if (version>=3) {
    pbfHeader.setUint8(8, pbfHeader.byteLength ); // header length / hashtable offset
    var features  =
      (use16BitOffsets ? 1 : 0) |
      (useExtendedHashTableOffset ? 128 : 0);
    pbfHeader.setUint8(9, features); // features
  }

  console.log("offset table size "+(pbfOffsetTableEntrySize * glyphs.length)+", chars "+glyphs.length);

  var pbfHashTable = new DataView(new ArrayBuffer(4 * hashtableSize));
  var n = 0, offsetSize = 0;
  hashes.forEach((glyphs,i) => {
    if (glyphs.length > 255) throw new Error("Too many hash entries!");
    if (!useExtendedHashTableOffset && offsetSize > 65535) throw new Error("hashtable offset too big! "+offsetSize);
    // if useExtendedHashTableOffset (an Espruino hack) then we use the value as the extra 8 bits of offset
    pbfHashTable.setUint8(n+0, useExtendedHashTableOffset ? (offsetSize>>16) : i); // value - this is redundant by the look of it?
    pbfHashTable.setUint8(n+1, glyphs.length); // offset table size
    pbfHashTable.setUint16(n+2, offsetSize & 65535, true/*LE*/); // offset in pbfOffsetTable
    n +=4 ;
    offsetSize += pbfOffsetTableEntrySize*glyphs.length;
  });

  var pbfOffsetTable = new DataView(new ArrayBuffer(pbfOffsetTableEntrySize * glyphs.length));
  n = 0;
  hashes.forEach(glyphs => {
    glyphs.forEach(glyph => {
      pbfOffsetTable.setUint16(n+0, glyph.ch, true/*LE*/); // codepoint size = 2
      if (use16BitOffsets)
        pbfOffsetTable.setUint16(n+2, glyph.dataOffset, true/*LE*/); // offset in data
      else
        pbfOffsetTable.setUint32(n+2, glyph.dataOffset, true/*LE*/); // offset in data
      n += pbfOffsetTableEntrySize;
    });
  });

  var pbfGlyphTable = new DataView(new ArrayBuffer(dataOffset));
  n = 0;
  glyphs.forEach(glyph => {
    pbfGlyphTable.setUint8(n+0, glyph.width); // width
    pbfGlyphTable.setUint8(n+1, glyph.height); // height
    pbfGlyphTable.setInt8(n+2, glyph.xStart); // left
    pbfGlyphTable.setInt8(n+3, glyph.yStart); // top
    pbfGlyphTable.setUint8(n+4, glyph.advance | (glyph.bpp==2?128:0)); // advance (actually a int8)
    n+=5;
    // now add data
    var bytes = bitsToBytes(glyph.bits, glyph.bpp);
    bytes.forEach(b => {
      pbfGlyphTable.setUint8(n++, parseInt(b.toString(2).padStart(8,0).split("").reverse().join(""),2));
    });
  });

  // finally combine
  if (1) {
    console.log(`Header     :\t0\t${pbfHeader.byteLength}`);
    console.log(`HashTable: \t${pbfHeader.byteLength}\t${pbfHashTable.byteLength}`);
    console.log(`OffsetTable:\t${pbfHeader.byteLength+pbfHashTable.byteLength}\t${pbfOffsetTable.byteLength}`);
    console.log(`GlyphTable: \t${pbfHeader.byteLength+pbfHashTable.byteLength+pbfOffsetTable.byteLength}\t${pbfGlyphTable.byteLength}`);
  }
  var fontFile = new Uint8Array(pbfHeader.byteLength + pbfHashTable.byteLength + pbfOffsetTable.byteLength + pbfGlyphTable.byteLength);
  fontFile.set(new Uint8Array(pbfHeader.buffer), 0);
  fontFile.set(new Uint8Array(pbfHashTable.buffer), pbfHeader.byteLength);
  fontFile.set(new Uint8Array(pbfOffsetTable.buffer), pbfHeader.byteLength + pbfHashTable.byteLength);
  fontFile.set(new Uint8Array(pbfGlyphTable.buffer), pbfHeader.byteLength + pbfHashTable.byteLength + pbfOffsetTable.byteLength);
  return fontFile;
}

/* Output PBF as a C file to include in the build

  options = {
    name : font name to use (no spaces!)
    path : path of output (with trailing slash)
    filename : filename (without .c/h)
  }
*/
Font.prototype.getPBFAsC = function(options) {
  var pbf = this.getPBF();
  require("fs").writeFileSync(options.path+options.filename+".h", `/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2023 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Generated by Espruino/libs/graphics/font/fontconverter.js
 *
 * Contains Custom Fonts
 * ----------------------------------------------------------------------------
 */

#include "jsvar.h"

JsVar *jswrap_graphics_setFont${options.name}(JsVar *parent);
`);
  require("fs").writeFileSync(options.path+options.filename+".c", `/*
* This file is part of Espruino, a JavaScript interpreter for Microcontrollers
*
* Copyright (C) 2023 Gordon Williams <gw@pur3.co.uk>
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
* ----------------------------------------------------------------------------
* This file is designed to be parsed during the build process
*
* Generated by Espruino/libs/graphics/font/fontconverter.js
*
* Contains Custom Fonts
* ----------------------------------------------------------------------------
*/

#include "${options.filename}.h"
#include "jswrap_graphics.h"

static const unsigned char pbfData[] = {
  ${pbf.map(b=>b.toString()).join(",").replace(/(............................................................................,)/g,"$1\n")}
};

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setFont${options.name}",
  "generate" : "jswrap_graphics_setFont${options.name}",
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Set the current font
*/
JsVar *jswrap_graphics_setFont${options.name}(JsVar *parent) {
  JsVar *pbfVar = jsvNewNativeString(pbfData, sizeof(pbfData));
  JsVar *r = jswrap_graphics_setFontPBF(parent, pbfVar);
  jsvUnLock(pbfVar);
  return r;
}
`);
};


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
