/*
http://www.discoverytelecom.eu/upload/iblock/7c7/m35_at_command.pdf
https://www.hobbielektronika.hu/forum/getfile.php?id=203810

*/
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

// Return GPS instance. callback is called whenever data is available!
exports.setGPSOn = function(isOn, callback) {
  Serial1.removeAllListeners();
  delete this.GPS;
  if (isOn) { 
    // Set up GPS    
    Serial1.setup(9600,{tx:PINS.GPS_RXD,rx:PINS.GPS_TXD});
    PINS.PWR_GPS_ON.set();
    return this.GPS = require("GPS").connect(Serial1, callback);
  } else {
    // Power off GPS
    PINS.PWR_GPS_ON.reset();
  }
};

/// Returns BME280 instance. callback when initialised. Call 'getData' to get the information
exports.setEnvOn = function(isOn, callback) {
  if (this.BME280) this.BME280.setPower(false);
  delete this.BME280;
  if (isOn) {
    var spi = new SPI();
    spi.setup({miso : PINS.BME_SDO, mosi : PINS.BME_SDI, sck: PINS.BME_SCK });
    if (callback) setTimeout(callback, 100, this.BME280); // wait for first reading
    return this.BME280 = require("BME280").connectSPI(spi, PINS.BME_CS);    
  }
};

/// Returns a LIS2MDL instance. callback when initialised. Then use 'read' to get data
exports.setMagOn = function(isOn, callback) {
  if (this.LIS2MDL) this.LIS2MDL.off();
  delete this.LIS2MDL;
  if (isOn) {
    var i2c = new I2C();
    i2c.setup({sda:PINS.LIS2MDL_SDA, scl:PINS.LIS2MDL_SCL});
    if (callback) setTimeout(callback, 100, this.LIS2MDL); // wait for first reading
    // {int:pin} isn't used yet, but at some point the module might include support
    return this.LIS2MDL = require("LIS2MDL").connectI2C(i2c, { int : PINS.LIS2MDL_INT });
  }
};

/// Returns a LIS3DH instance. callback when initialised. Then use 'read' to get data
exports.setAccelOn = function(isOn, callback) {
  if (this.LIS3DH) this.LIS3DH.off();
  delete this.LIS3DH;
  if (isOn) {
    var i2c = new I2C();
    i2c.setup({sda:PINS.LIS3DH_SDA, scl:PINS.LIS3DH_SCL});
    if (callback) setTimeout(callback, 100, this.LIS3DH); // wait for first reading
    // {int:pin} isn't used yet, but at some point the module might include support
    return this.LIS3DH = require("LIS3DH").connectI2C(i2c, { int : PINS.LIS3DH_INT1});  
  }
};

/// Returns a OPT3001 instance. callback when initialised. Then use 'read' to get data
exports.setOptoOn = function(isOn, callback) {
  if (this.OPT3001) this.OPT3001.off();
  delete this.OPT3001;
  if (isOn) {
    var i2c = new I2C();
    i2c.setup({sda:PINS.OPT_SDA, scl:PINS.OPT_SCL,bitrate:400000});
    if (callback) setTimeout(callback, 1000, this.OPT3001); // wait for first reading
    // {int:pin} isn't used yet, but at some point the module might include support
    return this.OPT3001 = require("OPT3001").connectI2C(i2c, { int : PINS.OPT_INT });
  }
};

// Turn cell connectivity on - will take around 8 seconds. Calls the `callback(usart)` when done. You then need to connect either SMS or QuectelM35 to the serial device `usart`
exports.setCellOn = function(isOn, callback) {
  if (isOn) {
    var that=this;
    return new Promise(function(resolve) {
      Serial1.removeAllListeners();
      Serial1.on('data', function(x) {}); // suck up any data that gets transmitted from the modem as it boots (RDY, etc)
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
      return new Promise(function(resolve){setTimeout(resolve,5000);});
    }).then(function() {
      Serial1.removeAllListeners();
      if (callback) setTimeout(callback,10,Serial1);
    });
  } else {
    PINS.PWR_GPRS_ON.reset(); // turn power off.
    if (callback) setTimeout(callback,1000);
  }
}


