I2C1.setup({"scl":D17,"sda":D16,bitrate:100000});
var lcd = require("HD44780").connectI2C(I2C1);
lcd.print("Hello ESP32!!");

/*
function isDevice(i2cBus,id) {
  try {
          return i2cBus.readFrom(id,1);
      }
      catch(err) {
          return -1;
      }
}
function detect(i2c,first,last) {
  first = first | 0;
      last = last | 0x77;
      var idsOnBus = Array();
      for (var id = first; id < last; id++) {
          if ( isDeviceOnBus(i2c,id) != -1)
              idsOnBus.push(id);
          }
      }
      return idsOnBus;
}
console.log(detect(I2C1));
*/
