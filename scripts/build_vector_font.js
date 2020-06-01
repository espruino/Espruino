#!/usr/bin/nodejs
/* This is a super hacky bit of code that takes a specially formatted SVG file
and creates an Espruino vector font from it.

*/

// SVG contents from http://forum.espruino.com/conversations/347368/#comment15288563
// Converted with https://nebbishhacker.github.io/svg2bangle/ tolerance 0
var polyImg = [
  "NaAzoTKjMqgxqjGsOaw5qjOqNKg0ozWiNqI3ozmiOKE2oA==",
  "ZDBiMWEzYTliO2Q8ZjxoO2k5aTdoNWY0ZDRiNWM3ZDZmNmc3ZzlmOmQ6YzljM2QyZjJnM2kzaDFmMA==",
  "JDAiMSEzIzMkMiYyJzMnNCY1IjghOiE8KTwpOiM6IzskOSg2KTQpMygxJjA=",
  "IUAhTCNMI0ImQidDJ0QmRSNFI0cnRyhIKEknSiNKI0wnTClLKkkqSClGKEUoRilEKUMoQSZA",
  "QUBBTENMQ0JFQkdDSEVIR0dJRUpDSkNMRkxIS0lKSkhKRElCSEFGQA==",
  "WUxZSlNKU0dXR1dFU0VTQllCWUBRQFFM",
  "YUBhTGNMY0dnR2dFY0VjQmlCaUA=",
  "FUARTBNMFkMZTBtMF0A=",
  "FGQSZRFnEWkSaxRsFWwXaxZpFWoUahNpE2cUZhVmFmcWbBhsGGQWZBZlF2UVZA==",
  "NGQyZTFnMWkyazRsNWw3azhpNmk1ajRqM2kzZzRmNWY2ZzhnN2U1ZA==",
  "IWAhbCNsI2ckZiVmJmcmaSVqJGojaSJrJGwlbCdrKGkoZydlJWQkZCJlI2UjYA==",
  "VGRSZVFnUWlSa1RsVWxXa1ZqVGpTaVNnVGZVZlZnU2dTaVhpWGdXZVVk",
  "YmxiZmFmYWRiZGJjY2FlYGZgZmJlYmRjZGRmZGZmZGZkbA==",
  "9GTyZfFn8Wnya/Rs9Wz1avRq82nzZ/Rm9Wb2Z/Zp9Wr1bPdr+Gn4Z/dl9WQ=",
  "gUCBTINMg0eIR4hMikyKQIhAiEWDRYNA",
  "kUCRQpJCkkqRSpFMlUyVSpRKlEKVQpVA",
  "qECoSKdKqEmmSqVKo0mkSqNIoUmiS6RMp0ypS6pJqkA=",
  "sUCxTLNMs0i0R7hMuky1RrpAuECzRrNA",
  "wUDBTMlMyUrDSsNA",
  "0UDRTNNM00TWStdK2kTaTNxM3EDaQNZI10jTQA==",
  "4UDhTONM40ToTOpM6kDoQOhI40A=",
  "AVABXANcA1IGUgdTB1UGVgNWA1gGWAhXCVUJUwhRBlA=",
  "FVATURJSEVQRWBJaE1sVXBhcF1oWWhRZE1cTVRRTFlIXUhlTGlUaVxlZF1oYXBpeHF0aWxtaHFgcVBtSGlEYUA==",
  "NFAyUTFTMVQyVjRXNlc3WDdZNlo0WjNZMVkyWzRcNlw4WzlZOVg4VjZVNFUzVDNTNFI2UjdTOVM4UTZQ",
  "I1ImUidTJ1UmViNWI1gmWCVXKFwqXCdXJ1goVylVKVMoUSZQIVAhXCNc",
  "QVBBUkVSRVxHXEdSS1JLUA==",
  "UVBRWVJbVFxXXFlbWllaUFhQWFhXWlhZVlpVWlNZVFpTWFNQ",
  "YVBlXGdca1BpUGZZY1A=",
  "cVB0XHZceFR6XHxcf1B9UHtYeVB3UHVYc1A=",
  "gVCJXItcg1A=",
  "i1CDXIFciVA=",
  "kVCVV5Vcl1yXV5tQmVCWVZNQ",
  "oVCpUKlSo1qpWqlcoVyhWqdSoVI=",
  "laCToZKikaSRqJKqk6uVrJmsmauVq5OqkqiSpJOilaGZoZuinKScqJuqmauZrJurnKqdqJ2knKKboZmg",
  "dUBzQXJCcURxSHJKc0t1THhMekt7SntGd0Z3SHlIeUl3SnZKdElzR3NFdEN2QndCeUN7QnpBeEA=",
  "NUAzQTJCMUQxSDJKM0s1TDhMOks7SjlJN0o2SjRJM0czRTRDNkI3QjlDO0I6QThA",
  "sWCxbLNss2q0abZsuGy1aLhktmSzaLNg",
  "gWCBbINsg2eEZoVmhmeGbIhsiGeHZYVkhGSCZYNlg2A=",
  "kWSTZJNskWw=",
  "kWCTYJNikWI=",
  "pWSlbKRuom+hb6Ftom2jbKNk",
  "o2ClYKVio2I=",
  "wWDDYMNswWw=",
  "0WTRbNNs02fUZtVm1mfWbNhs2GfZZtpm22fbbN1s3WfcZdpk2WTXZdVk1GTSZdNl02Q=",
  "4WThbONs42fkZuVm5mfmbOhs6GfnZeVk5GTiZeNl42Q=",
  "RmBGZUdlRWREZEJlQWdBaUJrRGxFbEdrRmlFakRqQ2lDZ0RmRWZGZ0ZsSGxIYA==",
  "AXQBfwN/A3cEdgV2BncGeQV6BHoDeQJ7BHwFfAd7CHkIdwd1BXQEdAJ1A3UDdA==",
  "FHQSdRF3EXkSexR8FXwXexZ5FXoUehN5E3cUdhV2FncWfxh/GHQWdBZ1F3UVdA==",
  "IXQhfCN8I3ckdiV2Jncodid1JXQkdCJ1I3UjdA==",
  "NXw3ezh5N3g1dzR3M3YzdzR2NXY3dzh2N3U1dDR0MnUxdzJ4NHk1eTZ6Nnk1ejR6MnkxejJ7NHw=",
  "QnBCeUN7RXxGfEZ6RXpEeURw",
  "UXRReVJ7VHxVfFd7VntWfFh8WHRWdFZ5VXpUelN5U3Q=",
  "YXRkfGZ8aXRndGV5Y3Q=",
  "gXSDdIl8h3w=",
  "iXSHdIF8g3w=",
  "mnSWfJV+k3+Sf5J9k32UfJh0",
  "kXSVfJZ6k3Q=",
  "oXSodKh2pHqoeqh8oXyheqV2oXY=",
  "BDACMQEzATkCOwQ8BDoDOQMzBDIGMgczBzkGOgQ6BDwGPAg7CTkJMwgxBjA=",
  "NDAyMTEzMzM0MjYyNzM3NDY1NDU0NzY3Nzg3OTY6NDozOTE5Mjs0PDY8ODs5OTk4ODY5NDkzODE2MA==",
  "FDARMhE0FDIUOhE6ETwZPBk6FjoWMA==",
  "RjBBN0E5STlJN0M3RjNGPEg8SDA=",
  "UTBRN1M3VDZWNlc3VzlWOlQ6UzlROVI7VDxWPFg7WTlZN1g1VjRUNFI1UzVTMlkyWTA=",
  "cTBxMncyczx1PHkyeTA=",
  "hDCCMYEzgTSCNoE4gTmCO4Q8hjyGOoQ6gzmDOIQ3hjeGNYQ1gzSDM4QyhjKHM4c0hjWGN4c4hzmGOoY8iDuJOYk4iDaJNIkziDGGMA==",
  "lDCSMZEzkTWSN5Q4ljiYN5c1ljaUNpM1kzOUMpYylzOXOZY6lDqTOZE5kjuUPJY8mDuZOZkzmDGWMA==",
  "cXR0fHZ8d3h4fHp8fXR7dHh8enx4dHZ0dHx2fHN0",
  "ESoTKhMsESw=",
  "ESATIBMoESg=",
  "ISAjICMkISQ=",
  "JCAmICYkJCQ=",
  "NCA2IDQsMiw=",
  "OCA6IDgsNiw=",
  "MCc7JzspMCk=",
  "MSM8IzwlMSU=",
  "WSBbIFQsUiw=",
  "UyBRIVAjUSVTJlMkUiNTIlQjUyRTJlUlViNVIQ==",
  "RCFCIkEkQiZEJ0UnRihFKUQpQyhBKEIqRCtFK0cqSChHJkUlRCVDJEQjRSNGJEgkRyJFIUQh",
  "ZiBkIWMjZCVpLGssZiVlI2YiZyNmJGImYShhKWIrZCxmLGgraSprJmkmaChmKmQqYyljKGQnaCVpI2gh",
  "cSBzIHMkcSQ=",
  "hiCEI4MmgymELIYvhC+CLIEpgSaCI4Qg",
  "0SXWJdYn0Sc=",
  "wS7DLsQsxCrCKsIs",
  "4SrjKuMs4Sw=",
  "+CD6IPMu8S4=",
  "oTSjNKM2oTY=",
  "oTqjOqM8oTw=",
  "sT6zPrQ8tDqyOrI8",
  "sjS0NLQ2sjY=",
  "yTLBNsk6yTjFNsk0",
  "0TPZM9k10TU=",
  "0TfZN9k50Tk=",
  "4TLpNuE64TjlNuE0",
  "9DDyMfEz8zP0MvYy9zP3NPY19TX0NvQ49jj2N/g2+TT5M/gx9jA=",
  "9Dr2OvY89Dw=",
  "sVCxX7Zftl2zXbNStlK2UA==",
  "1lDWX9Ff0V3UXdRS0VLRUA==",
  "w1DBUMheyl4=",
  "5VDhVuNW5VPnVulW",
  "8F34Xfhf8F8=",
  "tnC0cbNzs3Wyd7N2sXexeLN5snizerN8tH62f7d/t322fbV8tXq0eLN3s3i0d7V1tXO2crdyt3A=",
  "wXDDcMN/wX8=",
  "0nDUcdVz1XXWd9V213fXeNV51njVetV81H7Sf9F/0X3SfdN803rUeNV31XjUd9N103PSctFy0XA=",
  "5HPidOF24Xfjd+N25HXlduZ46HnqeOt263Xpdel26HfnduZ0",
  "AWADYAVjA2M=",
  "CUoLSwxLDkoPSA9FDkMMQQpABkAEQQJDAUYBSQJMBE4GTwpPDE4NTQtMCU0HTQVMBEsDSQNGBEQFQwdCCUILQwxEDUYNSAxJC0g=",
  "CEQHRAVFBEcESAVKB0sISwpKC0gLRAlECUgISQdJBkgGRwdGCEYJRwpF",
  "FEcTSRlJGEc=",
  "ArABsQGzArQCsQSxBLMCswK0BLQFswWxBLA=",
  "BPQG9Aj3Bvc=",
  "GfQX9BX3F/c=",
  "I/cm9Cn3J/cm9iX3",
  "Q/dD9UX1Rfc=",
  "R/dH9Un1Sfc=",
  "VfRU9VT3VfhV9Vf1V/dV91X4V/hY91j1V/Q=",
  "M/U18zf1OfM59Tf3NfUz9w==",
  "dGRyZXFncWlya3RsdWx3a3ZpdWp0anNpc2d0ZnVmdmd2bHVtdG1ybHFtcm50b3Vvd254bHhkdmR2ZXdldWQ=",
  "BoAEgQOCAoQCiAOKBIsGjAmMC4sKiQiKB4oFiQSHBIUFgweCCIIKgwuBCYA=",
  "lqKUo5Olk6eUqZaqmKqYqpqpm6eZp5iolqiVp5WllqSYpJmlm6Wao5ii",
  "6aHloeOi4qTiqOOq5avpq+ms5azjq+Kq4ajhpOKi46HloOmg",
  "6aHrouyk7Kjrqumr6azrq+yq7ajtpOyi66HpoA==",
  "5KLkquaq5qTopOim5qbmqOio56foquqq6afqpuqk6KI=",
  "9NDy0fHT8dzz3PPT9NL10vbT9tT11fXW99j32fba9Nr03Pbc+Nv52vnY99b31fjU+NP30fXQ",
  "EaQTpBOiEaI=",
  "Ea4TrhOmEaY=",
  "JKMipCGmIagiqiSrJasnqiioJqglqSSpI6gjpiSlJaUmpiimJ6QloySj",
  "QqJBo0KkQaZCqEGpQqpDqUWqRqhEqEOnQ6VEpEakR6VHp0aoRapHqUiqSalIqEmmSKRJo0iiR6NFokOj",
  "UqZaplqnUqc=",
  "UqhaqFqpUqk=",
  "YaljqWOvYa8=",
  "YaBjoGOmYaY=",
  "gaKBoIOgg6I=",
  "haKFoIegh6I=",
  "tKSxp7Sqtamzp7Wl",
  "t6S0p7equKm2p7il",
  "waXBp8enx6rJqsml",
  "8aD5oPmi8aI=",
  "FLEUuRa5FrE=",
  "EboZuhm8Ebw=",
  "RbBDsEGzQ7M=",
  "UbRRv1O/U7tSu1S8VbxXu1a7VrxYvFi0VrRWuVW6VLpTuVO0",
  "cbdzt3O5cbk=",
  "gryDvIS9hL6Dv4G/gb6CvoO9g76CvQ==",
  "tbS4t7W6tLm2t7S1",
  "srS1t7K6sbmzt7G1",
  "2bDbsNS80rw=",
  "27zbu9m727nbuNq32LfYuNq42rnYu9i8",
  "ybDLsMS8wrw=",
  "w7DCscKyw7HDtMK0wrXFtcW0xLTEsA==",
  "6bDrsOS84rw=",
  "9Lb0t/K48brxu/K99L72vvi9+bv3u/a89Lzzu/O69Ln1ufa49rY=",
  "9rT0tPSy9rI=",
  "ZsBhzGPMaMBnwmfHZcdkyWfJZ8xtzG3Kacppx2zHbMVpxWnCbcJtwA==",
  "AtAC3ATcBNIG0gjTCdUJ1wjZBtoE2gTcB9wJ2wraC9gL1ArSCdEH0A==",
  "AdUG1QbXAdc=",
  "cdNy0nnZeNo=",
  "eNJ503Lacdk=",
  "9Uz4TPpL+0r8SPxE+0L6QfhA9UDzQfJC8UTxSPJK80v1TPZK9EnzR/NF9EP2QvdC+UP6RfpH+Un3SvZK",
  "hdCD0YLSgdSB2ILag9uF3Ijch9qG2oTZg9eD1YTThtKH0onTitWK14nZh9qI3Irbi9qM2IzUi9KK0YjQ",
  "gdyK0IzQg9w=",
  "4dDh3OPc49Tm1OfV59fm2OPY49rm2ujZ6dfp1ejT5tLj0uPQ",
  "4fDh/+P/4/fk9uX25vfm+eX65Prj+eL75Pzl/Of76Pno9+f15fTk9OL14/Xj8A==",
  "ZORi5WPmZeZm52bpZepk6mPpY+pk6WbpZudj52HpYepj7GXsZ+tp7GrsbOtr6mnqaOlo52nmauZr52jnaOlt6W3nbOVq5GnkZ+Vl5A==",
  "cfV59Xn3cfc=",
  "dPJ28nb0dPQ=",
  "dPh2+Hb6dPo=",
  "hPSC9YH3gfmC+4T8hfyF+oT6g/mD94T2hfaG94b5hfqF/If7iPmI94f1hfQ=",
  "gfyG9Ij0g/w=",
  "YfRj9GP8Yfw=",
  "dMBywXHCcMRwyHHKcst0zHXMdc12znbNdc50znTPds93znfNdsx3zHnLesp4yXbKdcpzyXLHcsVzw3XCdsJ4w3rCecF3wA==",
  "dORy5XHncely63TsdO117nXtdO5z7nPvde927nbtdex363jpdul16nTqc+lz53TmdeZ253jnd+V15A==",
  "MaU3pTenMac=",
  "07DSsdKy07HTtNK00rXVtdW01LTUsA==",
  "6bfnuue767vruui66rfpt+m86rzqtw==",
  "4rDiseSx5LLjsuOz5LPktOK04rXkteW05bPksuSz5bLlseSw",
  "pCimKKYgpCA=",
  "qCepJaIhoSM=",
  "oiehJaghqSM=",
  "sSW5JbknsSc=",
  "tiK2KrQqtCI=",
  "kSCTI5QmlCmTLJEvky+VLJYpliaVI5Mg",
  "WiZYJ1cpWCtaLFoqWSlaKFspWipaLFwrXSlcJw==",
  "AYUBhgmGCYU=",
  "AYcBiAmICYc=",
  "UaBVp1WsV6xXp1ugWaBWpVOg",
  "ybfHuse7y7vLusi6yrfJt8m8yrzKtw==",
  "EbQZtBm2EbY=",
  "QXRBdkZ2RnQ=",
  "JKIkrCWsJaIkog==",
  "RCBELEUsRSBEIA=="
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
#define VF_SCALE 12
#define VF_OFFSET_Y (0)
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
for (var c=240;c<247;c++) {
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
    // 012345 correspond to 0=grave,1=acute,2=circumflex,3=tilde,4=umlaut,5=ring,6=i without dot
    char *chrMap = "AAAAAA  EEEEIIII NOOOOO  UUUUY  aaaaaa  eeeeiiii nooooo  uuuuy y";
    char *accMap = "012345  01240124 301234  01241  012345  01240124 301234  01241 4";
    if (ch>=192) {
      int i = ch-192;
      unsigned char chReplacement = (unsigned char)chrMap[i];
      if (chReplacement!=' ') {
        ch = chReplacement;
        if (accentPtr) {
          char acc = accMap[i];
          *accentX = 0;
          *accentY = 0;
          if (acc>='0' && acc<='5') {
            *accentPtr = &vfAccentPolys[vfAccentPolyIndices[acc-'0']];
            if (ch>='a' && ch<='z') { // lowercase
              *accentX = -2;
              *accentY = -4;
            } else { // uppercase
              *accentY = -8;
            }
            if (ch=='I') *accentX -= 3;
            if (ch=='i') {
              *accentX -= 2;
              return &vfAccentPolys[vfAccentPolyIndices[6]]; // use i without .
            }
          } else if (acc!=' ') {
            *accentPtr = vfGetCharPtr(acc, NULL,NULL,NULL);
            if (acc=='+') *accentY = -4;
          }
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
  x1 = x1<<4;
  y1 = y1<<4;
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
      poly[polyLen*2  ] = (short)(x1 + vx*size*16/VF_SCALE);
      poly[polyLen*2+1] = (short)(y1 + (vy+VF_OFFSET_Y)*size*16/VF_SCALE);
      polyLen++;
    }
    charPtr++;
  }
  graphicsFillPoly(gfx, polyLen, poly);
  return (unsigned int)(((w+1)*size*16/VF_SCALE+7)>>4);
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
  return (unsigned int)(((w+1)*size*16/VF_SCALE+7)>>4);
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
    w2 = vfDrawCharPtr(gfx, x1 + ((accentX*size*16/VF_SCALE)>>4), y1 + ((accentY*size*16/VF_SCALE)>>4), size, accentPtr);
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
