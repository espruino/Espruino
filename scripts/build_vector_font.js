#!/usr/bin/nodejs
/* This is a super hacky bit of code that takes a specially formatted SVG file
and creates an Espruino vector font from it.

*/

// SVG contents from http://forum.espruino.com/conversations/347368/#comment15288563
// Converted with https://nebbishhacker.github.io/svg2bangle/ tolerance 0
var polyImg = [
  "NqI3ozmiOKE2oA==",
  "ZDRiNWQ2ZjZnN2c5ZjpkOmQ8ZjxoO2k5aTdoNWY0",
  "ZDBiMWEzYTliO2Q8ZDpjOWM3ZDZjNWMzZDJmMmYw",
  "JDAkMiYyJzMnNCY1IjghOiE8KTwpOiM6IzskOSg2KTQpMygxJjA=",
  "IUAhTCNMI0A=",
  "I0ImQidDJ0QmRSNFI0cnRyhIKEknSiNKI0wnTClLKkkqSClGKEUoRilEKUMoQSZAI0A=",
  "Q0JFQkdDSEVIR0dJRUpDSkNMRkxIS0lKSkhKRElCSEFGQENA",
  "Q0BBQEFMQ0w=",
  "WUxZSlNKU0dXR1dFU0VTQllCWUBRQFFM",
  "YUBhTGNMY0dnR2dFY0VjQmlCaUA=",
  "E0wRTBVAF0A=",
  "F0AWQxlMG0w=",
  "FWYUZhNnE2kUahVqF2sVbBRsEmsRaRFnEmUUZBVkF2U=",
  "FmUWZBhkGGwWbBZrFWoWaRZnFWY=",
  "NWQ3ZThnNmc1Zg==",
  "NWw0bDJrMWkxZzJlNGQ1ZDVmNGYzZzNpNGo1ag==",
  "NWo2aThpN2s1bA==",
  "JGYlZiZnJmklaiRqImskbCVsJ2soaShnJ2UlZCRkImU=",
  "I2UjYCFgIWwjbCNrJGojaSNnJGY=",
  "VGRVZFdlWGdYaVNpU2dWZ1VmVGY=",
  "VGZTZ1NpVGpWaldrVWxUbFJrUWlRZ1JlVGQ=",
  "YmxiZmFmYWRiZGJjY2FlYGZgZmJlYmRjZGRmZGZmZGZkbA==",
  "9Wb0ZvNn82n0avVq9Wz0bPJr8WnxZ/Jl9GT1ZA==",
  "gUCBTINMg0eIR4hFg0WDQA==",
  "iECKQIpMiEw=",
  "kUCRQpJCkkqRSpFMlUyVSpRKlEKVQpVA",
  "p0ykTKJLoUmjSKRKo0mlSqZK",
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
  "A1IGUgdTB1UGVgNWA1gGWAhXCVUJUwhRBlADUA==",
  "GFAXUhlTGlUaVxlZF1oYXBpeHF0aWxtaHFgcVBtSGlE=",
  "IVAhXCNcI1A=",
  "NFAyUTFTMVQyVjRXNlc3WDdZNlo0WjRcNlw4WzlZOVg4VjZVNFUzVDNTNFI2UjZQ",
  "M1k0WjRcMlsxWQ==",
  "N1M2UjZQOFE5Uw==",
  "I1ImUidTJ1UmViNWI1gmWCVXKFwqXCdXJ1goVylVKVMoUSZQI1A=",
  "QVBBUkVSRVxHXEdSS1JLUA==",
  "WFBYWFdaWFlWWldcWVtaWVpQ",
  "V1xUXFJbUVlRUFNQU1hUWlNZVVpWWg==",
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
  "F1IWUhRTE1UTVxRZFloXWhhcFVwTWxJaEVgRVBJSE1EVUBhQ",
  "maGVoZOikqSSqJOqlauZq5mslayTq5KqkaiRpJKik6GVoJmg",
  "maGbopyknKibqpmrmaybq5yqnaidpJyim6GZoA==",
  "d0J2QnRDc0VzR3RJdkp3SnhMdUxzS3JKcUhxRHJCc0F1QHhA",
  "d0J5Q3tCekF4QA==",
  "d0p5SXlId0h3RntGe0p6S3hM",
  "N0I2QjRDM0UzRzRJNko3SjhMNUwzSzJKMUgxRDJCM0E1QDhA",
  "N0o4TDpLO0o5SQ==",
  "N0I4QDpBO0I5Qw==",
  "hGaFZoZnhmyIbIhnh2WFZIRkgmU=",
  "g2WDYIFggWyDbINnhGY=",
  "kWSTZJNskWw=",
  "kWCTYJNikWI=",
  "qkCqSalLp0ymSqhJp0qoSKhA",
  "pWSlbKRuom+hb6Ftom2jbKNk",
  "o2ClYKVio2I=",
  "sWCxbLNss2q1aLNos2A=",
  "tmSzaLZsuGy1aLhk",
  "wWDDYMNswWw=",
  "1GbVZtZn1mzYbNhn2WbVZNRk0mU=",
  "02XTZNFk0WzTbNNn1GY=",
  "2WbaZttn22zdbN1n3GXaZNlk12U=",
  "5GblZuZn5mzobOhn52XlZORk4mU=",
  "42XjZOFk4WzjbONn5GY=",
  "9Wb2Z/Zp9Wr1bPdr+Gn4Z/dl9WQ=",
  "RWZEZkNnQ2lEakVqR2tFbERsQmtBaUFnQmVEZEVkR2U=",
  "RmVGYEhgSGxGbEZrRWpGaUZnRWY=",
  "BHoFegZ5BncFdgR2AnUEdAV0B3UIdwh5B3sFfAR8Ans=",
  "A3sDfwF/AXQDdAN1BHYDdwN5BHo=",
  "FXoUehN5E3cUdhV2F3UVdBR0EnURdxF5EnsUfBV8F3s=",
  "FnsWfxh/GHQWdBZ1FXYWdxZ5FXo=",
  "JHYldiZ3KHYndSV0JHQidQ==",
  "I3UjdCF0IXwjfCN3JHY=",
  "NXw3ezh5N3g1dzR3M3YzdzR2NXY3dzh2N3U1dDR0MnUxdzJ4NHk1eTZ6Nnk1ejR6MnkxejJ7NHw=",
  "QnBCdEF0QXZCdkJ5Q3tFfEZ8RnpFekR5RHZGdkZ0RHREcA==",
  "VXpUelN5U3RRdFF5UntUfFV8V3s=",
  "VntWfFh8WHRWdFZ5VXo=",
  "ZHxmfGN0YXQ=",
  "ZnxkfGd0aXQ=",
  "gXSDdIl8h3w=",
  "iXSHdIF8g3w=",
  "mnSWfJV+k3+Sf5J9k32UfJh0",
  "kXSVfJZ6k3Q=",
  "oXSodKh2pHqoeqh8oXyheqV2oXY=",
  "BDwGPAg7CTkJMwgxBjAEMAQyBjIHMwc5BjoEOg==",
  "BDwCOwE5ATMCMQQwBDIDMwM5BDo=",
  "NDI2MjczNzQ2NTQ1NDc2Nzc4Nzk2OjQ6NDw2PDg7OTk5ODg2OTQ5MzgxNjA0MA==",
  "IjEhMyMzJDIkMA==",
  "MjExMzMzNDI0MA==",
  "MjsxOTM5NDo0PA==",
  "FDAUOhE6ETwZPBk6FjoWMA==",
  "RjBGPEg8SDA=",
  "QTdDN0gwRjA=",
  "UzdRN1EwWTBZMlMyUzVUNg==",
  "UjVUNFY0WDVZN1k5WDtWPFQ8VDpWOlc5VzdWNlQ2",
  "UjtROVM5VDpUPA==",
  "aDFpM2czZjJmMA==",
  "cTBxMncyczx1PHkyeTA=",
  "hDCCMYEzgTSCNoE4gTmCO4Q8hjyGOoQ6gzmDOIQ3hjeGNYQ1gzSDM4QyhjKGMA==",
  "hjCGMoczhzSGNYY3hziHOYY6hjyIO4k5iTiINok0iTOIMQ==",
  "ljiYN5Y2lDaTNZMzlDKWMpYwlDCSMZEzkTWSN5Q4",
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
  "ZipkKmMpYyhkJ2YmZiRkJWImYShhKWIrZCxmLA==",
  "ZixoK2kqayZpJmgoZylmKg==",
  "ZiBkIWMjZCVpLGssZiVlI2Yi",
  "cSBzIHMkcSQ=",
  "hiKEJIMmgymEK4Ythi+ELoIsgSmBJoIjhCGGIA==",
  "kSKTJJQmlCmTK5EtkS+TLpUslimWJpUjkyGRIA==",
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
  "8jHxM/Mz9DL0MA==",
  "9DD0MvYy9zP3NPY19TX0NvQ49jj2N/g2+TT5M/gx9jA=",
  "9Dr2OvY89Dw=",
  "sVCxX7Zftl2zXbNStlK2UA==",
  "1lDWX9Ff0V3UXdRS0VLRUA==",
  "w1DBUMheyl4=",
  "5VDhVuNW5VM=",
  "8V35Xflf8V8=",
  "tnC0cbNzs3Wyd7N2sXexeLN5snizerN8tH62f7d/t322fbV8tXq0eLN3s3i0d7V1tXO2crdyt3A=",
  "wXDDcMN/wX8=",
  "0nDUcdVz1XXWd9V213fXeNV51njVetV81H7Sf9F/0X3SfdN803rUeNV31XjUd9N103PSctFy0XA=",
  "5HPidOF24Xfjd+N25HU=",
  "5HPkdeV25njoeeh353bmdA==",
  "6XXpduh36HnqeOt263U=",
  "AWADYAZkBGQ=",
  "CEYHRgZHBkgHSQhJCkoISwdLBUoESARHBUUHRAhECkU=",
  "C0QLSAxJC0sISQlICUcIRglFCUQ=",
  "Ck8GTwROAkwBSQFGAkMEQQZACkAJQgdCBUMERANGA0kESwVMB00JTQ==",
  "CU0LTA1NDE4KTw==",
  "CkAMQQ5DD0UPSA5KDEsLSwxJDUgNRgxEC0MJQg==",
  "FEcTSRlJGEc=",
  "5VDpVudW5VM=",
  "ArAEsAWxBbMEtAK0ArMEswSxArE=",
  "ArQBswGxArA=",
  "BPQG9Aj3Bvc=",
  "GfQX9BX3F/c=",
  "I/cm9Cn3J/cm9iX3",
  "Q/dD9UX1Rfc=",
  "R/dH9Un1Sfc=",
  "VfRX9Fj1WPdX+FX4VfdX91f1VfU=",
  "VfhU91T1VfQ=",
  "M/U18zf1OfM59Tf3NfUz9w==",
  "dW10bXJscW1ybnRvdW93bnhseGR2ZHZldWZ2Z3ZpdWp2a3Zs",
  "dWZ0ZnNnc2l0anVqd2t1bHRscmtxaXFncmV0ZHVkd2U=",
  "BoAEgQOCAoQChQGFAYYChgKHAYcBiAKIA4oEiwaMCYwIigeKBYkEiAmICYcEhwSGCYYJhQSFBYMHggiCCYA=",
  "CIoJjAuLCok=",
  "CIIJgAuBCoM=",
  "mKqWqpSpk6eTpZSjlqKYopiklqSVpZWnlqiYqA==",
  "6aHloeOi4qTiqOOq5avpq+ms5azjq+Kq4ajhpOKi46HloOmg",
  "6aHrouyk7Kjrqumr6azrq+yq7ajtpOyi66HpoA==",
  "5KLkquaq5qI=",
  "5qTopOim5qbmqOio56foquqq6afqpuqk6KLmog==",
  "MqgyozOhNaA2oDaiNaI0ozSoM6o5qjmsMawxqg==",
  "9NL10vbT9tT11fXW9tf32PfZ9tr02vTc9tz42/na+dj41/fW99X41PjT99H10PTQ",
  "8tHx0/Hc89zz0/TS9NA=",
  "EaQTpBOiEaI=",
  "Ea4TrhOmEaY=",
  "JaMnpCimJqYlpQ==",
  "JKsiqiGoIaYipCSjJKUjpiOoJKk=",
  "JakmqCioJ6olqw==",
  "JKIloiWsJKw=",
  "mKKao5ulmaWYpA==",
  "mKiZp5unmqmYqg==",
  "QaZCpEGjQqJDo0WiRqREpEOlQ6dEqEaoRapDqUKqQalCqA==",
  "SaZIpEmjSKJHo0WiRqRHpUenRqhFqkepSKpJqUio",
  "W6BXp1esVaxVp1mg",
  "UaBVp1enU6A=",
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
  "FLEUtBG0EbYUthS5FrkWthm2GbQWtBax",
  "EboZuhm8Ebw=",
  "RbBDsEGzQ7M=",
  "VbpUulK7VLxVvFe7",
  "VrtWvFi8WLRWtFa5Vbo=",
  "U7tTv1G/UbRTtFO5VLo=",
  "cbdzt3O5cbk=",
  "gryDvIS9hL6Dv4G/gb6CvoO9g76CvQ==",
  "tbS4t7W6tLm2t7S1",
  "srS1t7K6sbmzt7G1",
  "2bDbsNS80rw=",
  "27zbu9m727nbuNq32LfYuNq42rnYu9i8",
  "ybDLsMS8wrw=",
  "w7TCtMK1xbXFtMS0xLDDsA==",
  "wrHDsMOxwrI=",
  "x7rHu8u7y7rIusq3ybc=",
  "ybfJvMq8yrc=",
  "6bDrsOS84rw=",
  "+L35u/e79rz2vg==",
  "9r72vPS887vzuvS59bn2uPa29Lb0t/K48brxu/K99L4=",
  "9rT0tPSy9rI=",
  "Y8xhzGbAaMA=",
  "Z8Bnx2THY8lnyWfMbcxtymnKacdsx2zFacVpwm3CbcA=",
  "BNIG0gjTCdUJ1wjZBtoE2gTcB9wJ2wraC9gL1ArSCdEH0ATQ",
  "BNAC0ALcBNw=",
  "AdUG1QbXAdc=",
  "cdNy0nnZeNo=",
  "eNJ503Lacdk=",
  "90L2QvRD80XzR/RJ9kr3SvhM9UzzS/JK8UjxRPJC80H1QPhA",
  "90L5Q/pF+kf5SfdK+Ez6S/tK/Ej8RPtC+kH4QA==",
  "h9KG0oTTg9WD14TZhtqH2ojchdyD24LagdiB1ILSg9GF0IjQ",
  "h9KJ04rViteJ2YfaiNyK24vajNiM1IvSitGI0A==",
  "gdyK0IzQg9w=",
  "4dDj0OPc4dw=",
  "49rm2ujZ6dfp1ejT5tLj0uPU5tTn1efX5tjj2A==",
  "5Prl+ub55vfl9uT24vXk9OX05/Xo9+j55/vl/OT84vs=",
  "4/vj/+H/4fDj8OP15Pbj9+P55Po=",
  "aeRp5mrma+dj52HpYepj7GXsZepk6mPpY+pk6W3pbeds5Wrk",
  "ZORi5WPmZeZm52bpZepl7Gfraexq7Gzra+pp6mjpaOdp5mnkZ+Vl5A==",
  "cfV59Xn3cfc=",
  "dPJ28nb0dPQ=",
  "dPh2+Hb6dPo=",
  "hfaE9oP3g/mE+oX6hfyE/IL7gfmB94L1hPSF9A==",
  "hfaG94b5hfqF/If7iPmI94f1hfQ=",
  "gfyG9Ij0g/w=",
  "YfRj9GP8Yfw=",
  "dMBywXHCcMRwyHHKcst0zHXMdc12znbNdc50znTPds93znfNdsx3zHbKdcpzyXLHcsVzw3XCdsJ3wA==",
  "dsp3zHnLesp4yQ==",
  "dsJ3wHnBesJ4ww==",
  "deR35Xjndud15g==",
  "dORy5XHncely63TsdO117nXtdO5z7nPvde927nbtdex16nTqc+lz53TmdeZ15A==",
  "dep26Xjpd+t17A==",
  "MaU3pTenMac=",
  "07TStNK11bXVtNS01LDTsA==",
  "0rHTsNOx0rI=",
  "57rnu+u767rouuq36bc=",
  "6bfpvOq86rc=",
  "4rDiseSx5LLjsuOz5LPktOK04rXkteW05bPksuSz5bLlseSw",
  "pCimKKYgpCA=",
  "qCepJaIhoSM=",
  "oiehJaghqSM=",
  "sSW5JbknsSc=",
  "tiK2KrQqtCI=",
  "QTdJN0k5QTk="
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
    char *chrMap = "AAAAAA  EEEEIIII NOOOOO  UUUUY  aaaaaa  eeeeiiiionooooo  uuuuy y";
    char *accMap = "012345  01240124 301234  01241  012345  01240124+301234  01241 4";
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
