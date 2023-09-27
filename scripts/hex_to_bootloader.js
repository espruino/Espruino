#!/usr/bin/env node
/* Take a hex file and turn it into Espruino commands that write
 to external flash. Special flash bootloaders (targets/nrf52_dfu/flash.c)
 can then take this and do the firmware update.
*/

var PAGESIZE = 4096;
var FLASH_OFFSET = 0x60300000;
var VERSION = 0xDEADBEEF; // VERSION! Use this to test firmware in JS land
var DEBUG = false;

if (process.argv.length!=3) {
  console.error("USAGE: hex_to_bootloader.js inputfile.hex");
  process.exit(1);
}

var inputFile = process.argv[2];

var hex = require("fs").readFileSync(inputFile).toString().split("\n");
function parseLines(dataCallback) {
  var addrHi = 0;
  hex.forEach(function(hexline) {
    if (DEBUG) console.log(hexline);
    var bytes = hexline.substr(1,2);
    var addrLo = parseInt(hexline.substr(3,4),16);
    var cmd = hexline.substr(7,2);
    if (cmd=="02") addrHi = parseInt(hexline.substr(9,4),16) << 4; // Extended Segment Address
    else if (cmd=="04") addrHi = parseInt(hexline.substr(9,4),16) << 16; // Extended Linear Address
    else if (cmd=="00") {
      var addr = addrHi + addrLo;
      var data = [];
      for (var i=0;i<16;i++) data.push(parseInt(hexline.substr(9+(i*2),2),16));
      dataCallback(addr,data);
    }
  });
}

function CRC32(data) {
  var crc = 0xFFFFFFFF;
  data.forEach(function(d) {
    crc^=d;
    crc=(crc>>>1)^(0xEDB88320&-(crc&1));
    crc=(crc>>>1)^(0xEDB88320&-(crc&1));
    crc=(crc>>>1)^(0xEDB88320&-(crc&1));
    crc=(crc>>>1)^(0xEDB88320&-(crc&1));
    crc=(crc>>>1)^(0xEDB88320&-(crc&1));
    crc=(crc>>>1)^(0xEDB88320&-(crc&1));
    crc=(crc>>>1)^(0xEDB88320&-(crc&1));
    crc=(crc>>>1)^(0xEDB88320&-(crc&1));
  });
  return ~crc;
}

function btoa(input) {
    var b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    var out = "";
    var i=0;
    while (i<input.length) {
      var octet_a = 0|input[i++];
      var octet_b = 0;
      var octet_c = 0;
      var padding = 0;
      if (i<input.length) {
        octet_b = 0|input[i++];
        if (i<input.length) {
          octet_c = 0|input[i++];
          padding = 0;
        } else
          padding = 1;
      } else
        padding = 2;
      var triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
      out += b64[(triple >> 18) & 63] +
             b64[(triple >> 12) & 63] +
             ((padding>1)?'=':b64[(triple >> 6) & 63]) +
             ((padding>0)?'=':b64[triple & 63]);
    }
    return out;
  }

// Work out addresses
var startAddress, endAddress = 0;
parseLines(function(addr, data) {
  if (startAddress === undefined || addr<startAddress)
    startAddress = addr;
  var end = addr + data.length;
  if (end > endAddress)
    endAddress = end;
});
console.log(`// Data from 0x${startAddress.toString(16)} to 0x${endAddress.toString(16)} (${endAddress-startAddress} bytes)`);
// Work out data
var headerLen = 16;
var binary = new Uint8Array(headerLen + endAddress-startAddress);
binary.fill(0); // actually seems to assume a block is filled with 0 if not complete
var bin32 = new Uint32Array(binary.buffer);
parseLines(function(addr, data) {
  var binAddr = headerLen + addr - startAddress;
  binary.set(data, binAddr);
  if (DEBUG) console.log("i",addr.toString(16).padStart(8,0), data.map(x=>x.toString(16).padStart(2,0)).join(" "));
  //console.log("o",new Uint8Array(binary.buffer, binAddr, data.length));
});

if (DEBUG) {
for (var i=0;i<binary.length;i+=16)
  console.log((i+startAddress-headerLen).toString(16).padStart(8,0), Array.from(new Uint8Array(binary.buffer, i, 16)).map(x=>x.toString(16).padStart(2,0)).join(" "));
process.exit(0);
}

/* typedef struct {
  uint32_t address;
  uint32_t size;
  uint32_t CRC;
  uint32_t version;
} FlashHeader; */
bin32[0] = startAddress;
bin32[1] = endAddress - startAddress;
bin32[2] = CRC32(new Uint8Array(binary.buffer, headerLen));
bin32[3] = VERSION; // VERSION! Use this to test ourselves
console.log("CRC 0x"+bin32[2].toString(16));

console.log(new Uint8Array(binary.buffer, 0,16));

console.log('var f = require("Flash");');
for (var i=0;i<binary.length;i+=4096)
  console.log('f.erasePage(0x'+(FLASH_OFFSET + i).toString(16)+');');
for (var i=0;i<binary.length;i+=256) {
  var l = binary.length-i;
  if (l>256) l=256;
  var chunk = btoa(new Uint8Array(binary.buffer, i, l));
  console.log('f.write(atob("'+chunk+'"), 0x'+(FLASH_OFFSET + i).toString(16)+');');
}

