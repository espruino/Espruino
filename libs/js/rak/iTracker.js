var PINS = {
  BME_CS : D2,
  BME_SDI : D3,
  BME_SCK : D4,
  BME_SDO : D5,
  PWR_GPRS_ON : D6, // 1=on, 0=off
  GPS_STANDBY : D7, // 1=powered, 0=standby (OK to leave open)
  GPS_RXD : D8, // 9600 baud
  GPS_TXD : D9, // 9600 baud
  PWR_GPS_ON : D10, // 1=on, 0=off
  LIS2MDL_SCL : D11,
  GPRS_TXD : D12,
  LIS2MDL_SDA : D13,
  GPRS_RESET : D14, 
  GPRS_PWRKEY : D15,
  LIS2MDL_INT : D16,
  BQ_EN : D17,
  LIS3DH_SCL : D18,
  LIS3DH_SDA : D19,
  GPRS_RXD : D20,
  // D21 is reset
  OPT_SDA : D26,
  OPT_INT : D22,
  OPT_SCL : D23,
  LIS3DH_INT1 : D25,
  LIS3DH_RES : D26,
  LIS3DH_INT2 : D27,
  SENSOR_DOUT1 : D28,
  SENSOR_DOUT2 : D29,
  TILT_DOUT : D30,
  GPS_RESET : D31 // 1=normal, 0=reset (internal pullup)
};

exports.GPS = function(callback) {
  if (callback) { 
    // Set up GPS    
    Serial1.removeAllListeners();
    Serial1.setup(9600,{tx:PINS.GPS_RXD,rx:PINS.GPS_TXD});
    PINS.PWR_GPS_ON.set();
    return require("GPS").connect(Serial1, callback);
  } else {
    // remove GPS
    Serial1.removeAllListeners();
    PINS.PWR_GPS_ON.reset();
  }
};

/// Returns BME280 instance. Call 'getData' to get the information
exports.BME = function() {
  var spi = new SPI();
  spi.setup({miso : PINS.BME_SDO, mosi : PINS.BME_SDI, sck: PINS.BME_SCK });
  return require("BME280").connectSPI(spi, PINS.BME_CS);
};

exports.mag = function() {
  var i2c = new I2C();
  i2c.setup({sda:PINS.LIS2MDL_SDA, scl:PINS.LIS2MDL_SCL});
  var m = require("LIS2MDL").connectI2C(i2c, {});
  var v = m.read(); 
  m.off();
  return v;
};

exports.accel = function() {
  // we could do tap detection/similar if supported by the module
  var i2c = new I2C();
  i2c.setup({sda:PINS.LIS3DH_SDA, scl:PINS.LIS3DH_SCL});
  var m = require("LIS3DH").connectI2C(i2c, {});
  var v = m.read(); 
  m.off();
  return v;
};

exports.light = function() {
  var i2c = new I2C();
  i2c.setup({sda:PINS.OPT_SDA, scl:PINS.OPT_SCL,bitrate:400000});
  var o = require("OPT3001").connectI2C(i2c);
  // need a delay here before reading - use OPT_INT?
  var v = o.read();
  o.off();
  return v;
};

// turn GSM on - returns a promise
exports.GSMon = function() {
  return new Promise(function(resolve) {
    Serial1.removeAllListeners();
    Serial1.on('data',x=>print(JSON.stringify(x)));
    Serial1.setup(115200,{tx:PINS.GPRS_TXD,rx:PINS.GPRS_RXD});
    PINS.PWR_GPRS_ON.reset();
    setTimeout(resolve,200);
  }).then(function() {
    PINS.PWR_GPRS_ON.set();
    return new Promise(function(resolve){setTimeout(resolve,200);});
  }).then(function() {
    PINS.GPRS_PWRKEY.set();
    return new Promise(function(resolve){setTimeout(resolve,2000);});
  }).then(function() {
    PINS.GPRS_PWRKEY.reset();
    return new Promise(function(resolve){setTimeout(resolve,1000);});
  }).then(function() {
    console.log("GSM on");
  });
}

// turn GSM off - returns a promise
exports.gsmoff = function() {
  return new Promise(function(resolve) {
    PINS.PWR_GPRS_ON.reset(); // turn power off.
    setTimeout(resolve,1000);
  }).then(function() {
    console.log("GSM off");
  });
}
