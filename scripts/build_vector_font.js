#!/usr/bin/nodejs
/* This is a super hacky bit of code that takes a specially formatted SVG file
and creates an Espruino vector font from it.

*/

// SVG contents from http://forum.espruino.com/conversations/347368/#comment15288563
// Converted with https://nebbishhacker.github.io/svg2bangle/
var polyImg = [
  "ZDRiNWY2ZzdnOWY6ZDpkPGg7aTloNWY0",
  "ZDBiMWEzYTliO2Q8ZDpjOWM3ZDZjNWMzZDJmMmYw",
  "JDAkMiYyJzQiOCE8KTwpOiM6IzskOSg2KTQoMSYw",
  "IUAhTCNMI0A=",
  "I0ImQidEJkUjRSNHJ0coSSdKI0ojTCdMKUsqSChFKEYpRChBJkAjQA==",
  "Q0JHQ0hFR0lDSkNMRkxJSkpISkRIQUZAQ0A=",
  "Q0BBQEFMQ0w=",
  "WUxZSlNKU0dXR1dFU0VTQllCWUBRQFFM",
  "YUBhTGNMY0dnR2dFY0VjQmlCaUA=",
  "E0wRTBVAF0A=",
  "F0AWQxlMG0w=",
  "FWYTZxNpF2sUbBJrEWcSZRRkF2U=",
  "FmUWZBhkGGwWbBVqFmkWZxVm",
  "NWQ3ZThnNmc1Zg==",
  "NWwyazFpMmU0ZDVkNWYzZzNpNWo=",
  "NWo2aThpN2s1bA==",
  "JGYmZyZpImslbCdrKGcnZSVkImU=",
  "I2UjYCFgIWwjbCRqI2kjZyRm",
  "VGRXZVhpU2lTZ1ZnVGY=",
  "VGZTZ1NpV2tVbFJrUWlSZVRk",
  "YmxiZmFmYWRiZGNhZmBmYmRjZGRmZGZmZGZkbA==",
  "9WbzZ/Np9Wr1bPRs8mvxZ/Jl9WQ=",
  "dW1xbHJudG93bnhseGN2Y3VldmZ2aHVpdmp2bA==",
  "dWVzZnNod2p0a3JqcWZyZHRjd2Q=",
  "gUCBTINMg0eIR4hFg0WDQA==",
  "iECKQIpMiEw=",
  "kkCRQJFCkkKSSpFKkUyVTJVKlEqUQpVClUCUQA==",
  "p0ykTKJLoUmjSKRKo0mmSg==",
  "sUCxTLNMs0i1RrNGs0A=",
  "uECzRrhMuky1RrpA",
  "wUDBTMlMyUrDSsNA",
  "0UDTQNNM0Uw=",
  "3EDaQNpM3Ew=",
  "4UDjQONM4Uw=",
  "6EzoSONA40Q=",
  "6kDoQOhM6kw=",
  "1krXStdI00DTRA==",
  "10rWSNpA2kQ=",
  "AVABXANcA1A=",
  "A1IGUgdTB1UGVgNWA1gGWAhXCVUIUQZQA1A=",
  "GFAXUhlTGlcZWRdaGl4cXRpbHFgcVBpR",
  "IVAhXCNcI1A=",
  "NFAyUTFTMlY2VzdYNlo0WjRcOFs5WThWNFUzVDRSNlI2UA==",
  "M1k0WjRcMlsxWQ==",
  "N1M2UjZQOFE5Uw==",
  "I1ImUidTJ1UmViNWI1gmWChXKVUoUSZQI1A=",
  "QVBBUkVSRVxHXEdSS1JLUA==",
  "WFBYWFdaWFlWWldcWVtaWVpQ",
  "V1xUXFJbUVlRUFNQU1hUWlNZVlo=",
  "ZVxnXGNQYVA=",
  "Z1xlXGlQa1A=",
  "dFx2XHNQcVA=",
  "fFx7WH1Qf1A=",
  "elx8XHlQeFQ=",
  "dlx1WHdQeVA=",
  "gVCJXItcg1A=",
  "i1CDXIFciVA=",
  "m1CXV5dclVyVV5lQ",
  "oVCpUKlSo1qpWqlcoVyhWqdSoVI=",
  "F1IUUxNVFFkXWhhcFVwSWhFYEVQTURVQGFA=",
  "90L0Q/NF9En3SvhM9UzzS/FI8UTzQfVA+EA=",
  "90L5Q/pF+Un3SvhM+0r8SPxE+kH4QA==",
  "d0J0Q3NFdEl3SnhMdUxySnFIcURzQXVAeEA=",
  "d0J5Q3tCeEA=",
  "d0p5SXlId0h3RntGe0p4TA==",
  "N0I0QzNFNEk3SjhMNUwySjFIMUQzQTVAOEA=",
  "N0o4TDtKOUk=",
  "N0I4QDtCOUM=",
  "hGaGZ4ZsiGyIZ4dlhGSCZQ==",
  "g2WDYIFggWyDbINnhGY=",
  "kWSTZJNskWw=",
  "kWCTYJNikWI=",
  "qkCqSalLp0ymSqhJp0qoSKhA",
  "pWSlbKRuoW+hbaNso2Q=",
  "o2ClYKVio2I=",
  "sWCxbLNss2q1aLNos2A=",
  "tmSzaLZsuGy1aLhk",
  "wWDDYMNswWw=",
  "1GbWZ9Zs2GzYZ9lm1WTSZQ==",
  "02XTZNFk0WzTbNNn1GY=",
  "2WbbZ9ts3WzdZ9xl2mTXZQ==",
  "5GbmZ+Zs6GzoZ+dl5GTiZQ==",
  "42XjZOFk4WzjbONn5GY=",
  "9Wb2Z/Vs92v4afdl9WQ=",
  "RWZDZ0NpR2tEbEJrQWdCZURkR2U=",
  "RmVGYEhgSGxGbEVqRmlGZ0Vm",
  "BHoGeQZ3AnUFdAd1CHkHewV8Ans=",
  "A3sDfwF/AXQDdAR2A3cDeQR6",
  "FXoTeRN3F3UUdBJ1EXkSexR8F3s=",
  "FnsWfxh/GHQWdBV2FncWeRV6",
  "JHYmdyh2JXQidQ==",
  "I3UjdCF0IXwjfCN3JHY=",
  "NXw3ezh5M3YzdzR2N3c4djV0MnUxdzZ6Nnk1ejJ5MXo0fA==",
  "QnBCdEF0QXZCdkJ5Q3tGfEZ6RHlEdkZ2RnREdERw",
  "VXpTeVN0UXRReVJ7VHxXew==",
  "VntWfFh8WHRWdFZ5VXo=",
  "ZHxmfGN0YXQ=",
  "ZnxkfGd0aXQ=",
  "gXSDdIl8h3w=",
  "iXSHdIF8g3w=",
  "mnSVfpJ/kn2UfJh0",
  "kXSVfJZ6k3Q=",
  "oXSodKh2pHqoeqh8oXyheqV2oXY=",
  "BDwIOwk5CTMIMQYwBDAEMgYyBzMHOQY6BDo=",
  "BDwCOwE5ATMCMQQwBDIDMwM5BDo=",
  "NDI2MjczNjU0NTQ3Njc3ODY6NDo0PDY8ODs5OTg2OTM4MTQw",
  "IjEhMyMzJDIkMA==",
  "MjExMzMzNDI0MA==",
  "MjsxOTM5NDo0PA==",
  "FDAUOhE6ETwZPBk6FjoWMA==",
  "RjBGN0E3QTlGOUY8SDxIOUk5STdIN0gw",
  "QTdDN0gwRjA=",
  "UzdRN1EwWTBZMlMyUzVUNg==",
  "UjVUNFg1WTdYO1Q8VDpWOlc5VzdWNlQ2",
  "UjtROVM5VDpUPA==",
  "aDFpM2czZjJmMA==",
  "cTBxMncyczx1PHkyeTA=",
  "hDCCMYEzgjaBOYI7hDyGPIY6hDqDOIQ3hjeGNYQ1gzOEMoYyhjA=",
  "hjCGMoczhjWGN4c4hjyIO4k5iDaJM4gx",
  "ljiYN5Q2kzWTM5QyljKWMJIxkTOSN5Q4",
  "ljyYO5k5mTOYMZYwljKXM5c1ljaXN5c5ljqUOpQ8",
  "kjuROZM5lDqUPA==",
  "ETIUMBQyETQ=",
  "kVCVV5dXk1A=",
  "dHx2fHN0cXQ=",
  "dnx0fHZ0eHQ=",
  "eHx6fHh0dnQ=",
  "enx4fHt0fXQ=",
  "ESoTKhMsESw=",
  "ESATIBMoESg=",
  "ISAjICMkISQ=",
  "JCAmICYkJCQ=",
  "NCA2IDQsMiw=",
  "OCA6IDgsNiw=",
  "MCc7JzspMCk=",
  "MSM8IzwlMSU=",
  "WSBbIFQsUiw=",
  "UCNRIVMgUyJSI1MkUyZRJQ==",
  "ViNVIVMgUyJUI1MkUyZVJQ==",
  "VylYJ1omWihZKVoqWixYKw==",
  "XSlcJ1omWihbKVoqWixcKw==",
  "QyhEKUQrQipBKA==",
  "RiRFI0UhRyJIJA==",
  "RCBFIEUsRCw=",
  "RCFCIkEkQiZEJ0QlQyREIw==",
  "RSVFJ0YoRSlFK0cqSChHJg==",
  "ZiZoJWkjaCFmIGYiZyNmJA==",
  "ZipkKmMpZCdmJmYkYiZhKGIrZiw=",
  "ZixpKmsmaSZmKg==",
  "ZiBkIWMjaSxrLGUjZiI=",
  "cSBzIHMkcSQ=",
  "hiKDJoMphi2GL4IsgSmCI4Yg",
  "kSKUJpQpkS2RL5UslimVI5Eg",
  "tCK0JbElsSe0J7Qqtiq2J7knuSW2JbYi",
  "0SXXJdcn0Sc=",
  "pCimKKYgpCA=",
  "pCKiIaEjoyShJaInpCY=",
  "piKoIakjpySpJagnpiY=",
  "wS7DLsQqwirCLA==",
  "4SrjKuMs4Sw=",
  "+CD6IPMu8S4=",
  "oTSjNKM2oTY=",
  "oTqjOqM8oTw=",
  "sT6zPrQ6sjqyPA==",
  "sjS0NLQ2sjY=",
  "yTLBNsk6yTjFNsk0",
  "0TPZM9k10TU=",
  "0TfZN9k50Tk=",
  "4TLpNuE64TjlNuE0",
  "8jHxM/Mz9DL0MA==",
  "9DD0MvYy9zP2NfQ29Dj2OPY3+Db5NPgx9jA=",
  "9Dr2OvY89Dw=",
  "sVCxX7Zftl2zXbNStlK2UA==",
  "1lDWX9Ff0V3UXdRS0VLRUA==",
  "w1DBUMheyl4=",
  "5VDhVuNW5VM=",
  "8V35Xflf8V8=",
  "tnC0cbJ3s3axd7F4s3myeLR+tn+3f7d9tXy1erN3s3i1dbVzt3K3cA==",
  "wXDDcMN/wX8=",
  "0nDUcdZ31XbXd9d41XnWeNR+0n/Rf9F903zTetV31XjTddNz0XLRcA==",
  "5HPidOF343fkdQ==",
  "5HPkdeZ46Hnod+Z0",
  "6XXoeep463U=",
  "AWADYAZkBGQ=",
  "CEYGRwZICkoHSwVKBEcFRQdECkU=",
  "C0QLSAxJC0sISQlICEYJRA==",
  "Ck8GTwROAkwBSQJDBkAKQAlCB0IERANGA0kFTAlN",
  "CU0LTA1NCk8=",
  "CkAOQw9FD0gOSgxLC0sNSA1GC0MJQg==",
  "JVcoXCpcJ1c=",
  "FEcTSRlJGEc=",
  "5VDpVudW5VM=",
  "B4IEgwSEBoQGhQSFBIcGhwaIBIgEiQeKCIwFjAKKAogAiACHAocChQCFAIQChAKCBYAIgA==",
  "B4oIjAqLC4oJiQ==",
  "B4IIgAuCCYM=",
  "A7AFsAaxBrMFtAO0A7MFswWxA7E=",
  "A7QCswKxA7A=",
  "BPQG9Aj3Bvc=",
  "GfQX9BX3F/c=",
  "I/cm9Cn3J/cm9iX3",
  "Q/dD9UX1Rfc=",
  "R/dH9Un1Sfc=",
  "VfRX9Fj1WPdX+FX4VfdX91f1VfU=",
  "VfhU91T1VfQ=",
  "M/U18zf1OfM59Tf3NfUz9w=="
];

var charData = [];
var firstChar=255, lastChar=0;
var maxPolyLen = 0;
polyImg.forEach(function(polyb64) {
  let buf = new Buffer(polyb64, 'base64');
  var minx=1000,maxx=-1,miny=1000,maxy=-1;
  var poly = [];
  for (var i=0;i<buf.length;i+=2) {
    var x = buf.readUInt8(i);
    var y = buf.readUInt8(i+1);
    if (x<minx) minx=x;
    if (y<miny) miny=y;
    if (x>maxx) maxx=x;
    if (y>maxy) maxy=y;
    poly.push((x&15)+((y&15)<<4));
  }
  if (poly.length > maxPolyLen)
    maxPolyLen = poly.length;
  var cx = minx>>4;
  var cy = miny>>4;
  var dx = maxx>>4;
  var dy = maxy>>4;
  if (cx!=dx || cy!=dy) console.log("Char straddles boundary!");
  var charNumber = cx + (cy*16);
  if (charNumber < 240 && charNumber > lastChar) lastChar = charNumber;
  if (charNumber < firstChar) firstChar = charNumber;

  if (charData[charNumber]===undefined)
    charData[charNumber] = [];
  charData[charNumber].push(poly);
});

function outputCharCode(c, char) {
  vector_font_c += `// ${c}\n`;
  if (char!==undefined && char.length) {
    var elements = 0;
    char.forEach((poly,n) => {
      var last = n == char.length-1;
      vector_font_c += poly.join(",")+
        ","+(last?"VF_END_OF_CHAR":"VF_END_OF_POLY")+",\n";
      elements += poly.length+1;
    });
    return elements;
  } else {
    vector_font_c += "VF_END_OF_CHAR,\n";
    return 1;
  }
}

var vector_font_c = `/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2020 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Machine generated vector font header (by build_vector_font.sh)
 * ----------------------------------------------------------------------------
 */

#ifndef NO_VECTOR_FONT
#include "graphics.h"

const uint8_t vfFirstChar = ${firstChar};
const uint8_t vfLastChar = ${lastChar};
#define VF_END_OF_POLY 0xFE
#define VF_END_OF_CHAR 0xFF
#define VF_OFFSET_Y (2)
static const uint8_t vfPolys[] IN_FLASH_MEMORY = {
`;
for (var c=firstChar;c<=lastChar;c++) {
  var char = charData[c];
  outputCharCode(c, char);
}

var accentIndices = [];
vector_font_c += `};

static const uint8_t vfAccentPolys[] IN_FLASH_MEMORY = {
`;
var n = 0;
for (var c=240;c<246;c++) {
  var char = charData[c];
  accentIndices[c-240] = n;
  n += outputCharCode(c, char);
}
vector_font_c += `
};
static const uint8_t vfAccentPolyIndices[] = {${accentIndices.join(",")}};

static const uint8_t *vfGetCharPtr(char sch, const uint8_t **accentPtr, int *accentX, int *accentY) {
  unsigned char ch = (unsigned char)sch;
  if (ch>=192) {
    // 012345 correspond to 0=grave,1=acute,2=circumflex,3=tilde,4=umlaut,5=ring
    char *chrMap = "AAAAAAACEEEEIIIIDNOOOOOxOUUUUYIBaaaaaaaceeeeiiiionooooo-ouuuuyIy";
    char *accMap = "012345E,01240124-301234 /01241o 012345e,01240124+301234:/01241o4";
    if (ch>=192) {
      int i = ch-192;
      ch = (unsigned char)chrMap[i];
      if (accentPtr) {
        char acc = accMap[i];
        *accentX = 0;
        *accentY = 0;
        if (acc>='0' && acc<='5') {
          *accentPtr = &vfAccentPolys[vfAccentPolyIndices[acc-'0']];
          if (ch>='a' && ch<='z') { // lowercase
            *accentX = -2;
            *accentY = -5;
          } else {
            *accentY = -10;
          }
        } else if (acc!=' ') {
          *accentPtr = vfGetCharPtr(acc, NULL,NULL,NULL);
          if (acc=='E') *accentX = 8;
          if (acc=='e') *accentX = 4;
          if (acc==',') *accentX = 4;
          if (acc=='+') *accentY = -4;
        }

      }
    }
  }
  if (ch<vfFirstChar || ch>vfLastChar) return NULL;
  if (ch==vfFirstChar) return vfPolys;
  int charCounter = ((uint8_t)ch) - vfFirstChar;
  for (unsigned int p=0;p < sizeof(vfPolys);p++) {
    if (vfPolys[p] == VF_END_OF_CHAR) {
      charCounter--;
      if (charCounter==0)
        return &vfPolys[p+1];
    }
  }
  return 0; // we went past the end
}

// prints character, returns width
unsigned int vfDrawCharPtr(JsGraphics *gfx, int x1, int y1, int size, const uint8_t *charPtr) {
  short poly[${maxPolyLen*2}];
  int polyLen = 0;
  int w = 0;
  while (*charPtr!=VF_END_OF_CHAR) {
    if (*charPtr==VF_END_OF_POLY) {
      graphicsFillPoly(gfx, polyLen, poly);
      polyLen = 0;
    } else {
      uint8_t vertex = *charPtr;
      int vx = vertex&15;
      int vy = vertex>>4;
      if (vx>w) w=vx;
      poly[polyLen*2  ] = (short)(x1 + ((vx*size+8)>>4));
      poly[polyLen*2+1] = (short)(y1 + ((vy*size+8)>>4) + VF_OFFSET_Y);
      polyLen++;
    }
    charPtr++;
  }
  graphicsFillPoly(gfx, polyLen, poly);
  return (unsigned int)(((w+1)*size+7)>>4);
}

// returns the width of a character
unsigned int graphicsVectorCharWidth(JsGraphics *gfx, unsigned int size, char ch) {
  NOT_USED(gfx);
  const uint8_t *p = vfGetCharPtr(ch, NULL,NULL,NULL);
  if (!p) return (unsigned int)(size/2);
  int w = 0;
  while (*p!=VF_END_OF_CHAR) {
    if (*p!=VF_END_OF_POLY) {
      uint8_t vertex = *p;
      int vx = vertex&15;
      if (vx>w) w=vx;
    }
    p++;
  }
  return (unsigned int)(((w+1)*size+7)>>4);
}

// prints character, returns width
unsigned int graphicsFillVectorChar(JsGraphics *gfx, int x1, int y1, int size, char ch) {
  const uint8_t *accentPtr = 0;
  int accentX, accentY;
  const uint8_t *charPtr = vfGetCharPtr(ch, &accentPtr, &accentX, &accentY);
  if (!charPtr) return (unsigned int)(size/2);
  unsigned int w = vfDrawCharPtr(gfx, x1, y1, size, charPtr);
  unsigned int w2 = 0;
  if (accentPtr)
    w2 = vfDrawCharPtr(gfx, x1 + ((accentX*size)>>4), y1 + ((accentY*size)>>4), size, accentPtr);
  return (w2>w)?w2:w;
}

#endif
`;

var vector_font_h = `/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2020 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Machine generated vector font header (by build_vector_font.sh)
 * ----------------------------------------------------------------------------
 */

#ifndef NO_VECTOR_FONT
#include "graphics.h"

// returns the width of a character
unsigned int graphicsVectorCharWidth(JsGraphics *gfx, unsigned int size, char ch);
// prints character, returns width
unsigned int graphicsFillVectorChar(JsGraphics *gfx, int x1, int y1, int size, char ch);
#endif
`;


require("fs").writeFileSync(__dirname + "/../libs/graphics/vector_font.c", vector_font_c);
require("fs").writeFileSync(__dirname + "/../libs/graphics/vector_font.h", vector_font_h);
