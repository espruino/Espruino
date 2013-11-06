I2C1.setup({scl:B6, sda:B7})

LCD.prototype.write = function(x, c) {
  var f = (x&0xF0) |8| ((c==undefined)?1:0); print(f);
  I2C1.writeTo(0x27, f);
  I2C1.writeTo(0x27, f | 4);
  I2C1.writeTo(0x27, f);
  var f = ((x<<4)&0xF0) |8| ((c==undefined)?1:0); print(f);
  I2C1.writeTo(0x27, f);
  I2C1.writeTo(0x27, f | 4);
  I2C1.writeTo(0x27, f);
};

function LCD(i2c) {
 this.i2c = i2c;
 this.write(0x33,1);
 this.write(0x32,1);
 this.write(0x28,1);
 this.write(0x0C,1);
 this.write(0x06,1);
 this.write(0x01,1);
}
LCD.prototype.write = function(x, c) {
  var a = (x&0xF0) |8| ((c==undefined)?1:0);
  var b = ((x<<4)&0xF0) |8| ((c==undefined)?1:0);
  this.i2c.writeTo(0x27, [a,a|4,a,b,b|4,b]);
};
