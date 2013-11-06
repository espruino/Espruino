function LCD() {
  this.DC = B6;
  this.CE = B7;
  this.RST = B8;
  digitalPulse(this.RST, 0, 10); // pulse reset low
  SPI1.setup({ baud: 1000000, sck:B3, mosi:B5 });
}
LCD.prototype.cmd = function (cmd) {
 digitalWrite(this.DC,0);
 SPI1.send(cmd, this.CE);
};
LCD.prototype.data = function (data) {
 digitalWrite(this.DC,1);
 SPI1.send(data, this.CE);
};
LCD.prototype.init = function () {
 this.cmd(0x21); // fnset extended
 this.cmd(0x80 | 0x40); // setvop (experiment with 2nd val to get the right contrast)
 this.cmd(0x14); // setbias 4
 this.cmd(0x04 | 0x02); // temp control
 this.cmd(0x20); // fnset normal
 this.cmd(0x08 | 0x04); // dispctl normal
 this.pixels = new Uint8Array(6*84);
};
LCD.prototype.setPixel = function (x,y,c) {
 var yp = y&7;
 y>>=3;
 this.cmd(0x40 | y); // Y addr
 this.cmd(0x80 | x); // X addr
 var p = x+y*84;
 if (c) this.pixels[p] |= 1<<yp;
 else this.pixels[p] &= ~(1<<yp);
 this.data(this.pixels[p]);
 this.cmd(0x40); // revert
};
LCD.prototype.clear = function () {
 this.pixels = new Uint8Array(6*84);
 for (var i=0;i<6;i++) {
   this.cmd(0x40 | i); // Y addr
   this.cmd(0x80); // X addr
  for (var x=0;x<84;x++) this.data(0);
 }
 this.cmd(0x40); // revert
};
LCD.prototype.blit = function () {
 for (var i=0;i<6;i++) {
   this.cmd([0x40 | i, 0x80]); // Y addr, X addr
   this.data(new Uint8Array(this.pixels.buffer,i*84,84));
 }
 this.cmd(0x40); // revert
};
LCD.prototype.clear = function () {
 this.pixels = new Uint8Array(6*84);
 this.blit();
};


// was working - now not??
var lcd = new LCD();
lcd.init()
for (var i=0;i<48;i++) lcd.setPixel(47-i,i,1)


for (var y=0;y<48;y++) {
 line="";
 for (var x=0;x<48;x++) {
  var Xr=0;
  var Xi=0;
  var Cr=(4.0*x/48)-2.0;
  var Ci=(4.0*y/48)-2.0;
  var i=0;
  while ((i<16) && ((Xr*Xr+Xi*Xi)<4)) {
   var t=Xr*Xr - Xi*Xi + Cr;
   Xi=2*Xr*Xi+Ci;
   Xr=t;
   i++;
  }
  lcd.setPixel(x,y,i&1);
 }
}

// ---------------------------------------------------------------------------------

var LCD = Graphics.createArrayBuffer(84,48,1,{vertical_byte:true});
LCD.init = function () {
  this.DC = B6;
  this.CE = B7;
  this.RST = B8;
  SPI1.setup({ baud: 200000, sck:B3, mosi:B5 });
  digitalPulse(this.RST, 0, 10); // pulse reset low
  setTimeout(function() {
    digitalWrite(this.DC,0); // cmd
    SPI1.send(
        [0x21, // fnset extended
        0x80 | 0x40, // setvop (experiment with 2nd val to get the right contrast)
        0x14, // setbias 4
        0x04 | 0x02, // temp control
        0x20, // fnset normal
        0x08 | 0x04], this.CE); // dispctl normal
  }, 100);
};
LCD.flip = function () {
 for (var i=0;i<6;i++) {
  digitalWrite(this.DC,0); // cmd
  SPI1.send(0x40|i, this.CE); // Y addr
  SPI1.send(0x80, this.CE); // X addr
  digitalWrite(this.DC,1); // data
  SPI1.send(new Uint8Array(this.buffer,i*84,84+2), this.CE);
 }
 // Why +2 in SPI.send? Maybe it needs some time to sort itself out
}

LCD.init();
LCD.clear();LCD.setFontVector(30);LCD.drawString("Hello");LCD.flip();



