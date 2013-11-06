SPI2.setup({baud:1600000, mosi:B15});
leds = Graphics.createArrayBuffer(16,16,24,{zigzag:true});
leds.flip = function() { SPI2.send4bit(leds.buffer, 0b0001, 0b0011); }

i=0;
function step() {
  i++; if (i>90) i = 0;
  leds.clear();
  leds.setColor(0,0,0.1);
  leds.setFontVector(20);
  leds.drawString("Hello",-(i-35),-5);
  leds.flip();
}
setInterval(step,50);

// Pong
var leds = Graphics.createArrayBuffer(16,16,24,{zigzag:true});
leds.flip = function() { SPI2.send4bit(leds.buffer, 0b0001, 0b0011); }

function onInit() {
  I2C1.setup({scl:B6,sda:B7});
  I2C1.writeTo(0x52, [0xF0,0x55])                                                                                                                   ;
  I2C1.writeTo(0x52, [0xFB,0x00])
  SPI2.setup({baud:1600000, mosi:B15});
}
onInit();

function read() {
  var d = I2C1.readFrom(0x52, 6);
  I2C1.writeTo(0x52, 0);
  bx = bx + dx;
  by = by + dy;
  if (by<0) {
    by=0;
    dy=Math.abs(dy);
  }
  if (by>15) {
   by=15;
   dy=-Math.abs(dy);
  }
  if (bx<0) {
    bx=0;
    dx=Math.abs(dx);
  }
  if (bx>15) {
   bx=15;
   dx=-Math.abs(dx);
  }

  var b1 = d[0]/16;
  var b2 = d[1]/16;
  leds.clear();
  leds.setColor(0.1,0,0);
  leds.fillRect(0,b1-3,0,b1+3);
  leds.fillRect(15,b2-3,15,b2+3);
  leds.setColor(0,0.1,0.1);
  leds.fillRect(bx,by,bx,by);
  leds.flip();
}
var bx = 12;
var by = 13.6;
var dx = 1;
var dy = 0.8;


onInit();
setInterval(read,100);
