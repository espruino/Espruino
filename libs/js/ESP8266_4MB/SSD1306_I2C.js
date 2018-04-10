/* Copyright (c) 2014 Sam Sykes, Gordon Williams. See the file LICENSE for copying permission. */
/* 
Module for the SSD1306 OLED controller in displays like the Crius CO-16
```
function go(){
 // write some text
 g.drawString("Hello World!",2,2);
 // write to the screen
 g.flip(); 
}
// I2C
I2C1.setup({scl:B6,sda:B7});

var g = require("SSD1306").connect(I2C1, go);
// or
var g = require("SSD1306").connect(I2C1, go, { address: 0x3C });

```
*/
var C = {
 OLED_WIDTH                 : 128,
 OLED_CHAR                  : 0x40,
 OLED_CHUNK                 : 128
};

// commands sent when initialising the display
var extVcc=false; // if true, don't start charge pump 
var initCmds = new Uint8Array([ 
             0xAe,     // 0 disp off
             0xD5,     // 1 clk div
             0x80,     // 2 suggested ratio
             0xA8, 63, // 3 set multiplex, height-1
             0xD3,0x0, // 5 display offset
             0x40,     // 7 start line
             0x8D, extVcc?0x10:0x14, // 8 charge pump
             0x20,0x0,   // 10 memory mode
             0xA1,       // 12 seg remap 1
             0xC8,       // 13 comscandec
             0xDA, 0x12, // 14 set compins, height==64 ? 0x12:0x02,
             0x81, extVcc?0x9F:0xCF, // 16 set contrast
             0xD9, extVcc?0x22:0xF1, // 18 set precharge
             0xDb, 0x40, // 20 set vcom detect
             0xA4,       // 22 display all on
             0xA6,       // 23 display normal (non-inverted)
             0xAf        // 24 disp on
            ]);
// commands sent when sending data to the display
var flipCmds = [
     0x21, // columns
     0, C.OLED_WIDTH-1,
     0x22, // pages
     0, 7 /* (height>>3)-1 */];
function update(options) {
  if (options) {
    if (options.height) {
      initCmds[4] = options.height-1;
      initCmds[15] = options.height==64 ? 0x12 : 0x02;
      flipCmds[5] = (options.height>>3)-1;
    }
    if (options.contrast!==undefined) initCmds[17] = options.contrast;
  }
}

exports.connect = function(i2c, callback,options) {
  update(options);
  var oled = Graphics.createArrayBuffer(C.OLED_WIDTH,initCmds[4]+1,1,{vertical_byte : true});

  var addr = 0x3C;
  
  if(options) {
    if (options.address) addr = options.address;  
    if (options.rst) digitalPulse(options.rst, 0, 10); 
  }
  
  setTimeout(function() {
    //initCmds.forEach(function(d) {console.log('0x'+d.toString(16)+',');i2c.writeTo(addr, [0,d]);});;
    initCmds.forEach(function(d) {i2c.writeTo(addr, [0,d]);});;
  }, 50);

  if (callback !== undefined) setTimeout(callback, 100);

  oled.flip = function() { 
    flipCmds.forEach(function(d) {i2c.writeTo(addr, [0,d]);});;
    var chunk = new Uint8Array(C.OLED_CHUNK+1);

    chunk[0] = C.OLED_CHAR;
    for (var p=0; p<this.buffer.length; p+=C.OLED_CHUNK) {
      chunk.set(new Uint8Array(this.buffer,p,C.OLED_CHUNK), 1);
      i2c.writeTo(addr, chunk);
    } 
  };

  // reconnect oled, only init commands
  oled.reconnect = function(){
    initCmds.forEach(function(d) {i2c.writeTo(addr, [0,d]);});
  }
  // set contrast, 0..255
  oled.setContrast = function(c) { console.log("c:"+c.toString(16));i2c.writeTo(addr, 0, 0x81, c); };

  // set display off
  oled.off = function() { i2c.writeTo(addr, 0, 0xAE); }

  // set display on
  oled.on = function() { i2c.writeTo(addr, 0, 0xAF); }

  // return graphics
  return oled;
};
