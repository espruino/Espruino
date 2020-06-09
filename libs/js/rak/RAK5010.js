var PINS = {
  LTE_RXD : D8,
  LTE_CTS : D11,
  LTE_TXD : D6,
  LTE_RTS : D7,
  LTE_STATUS : D18,
  LTE_WDISABLE : D29,
  LTE_DTR : D26,
  LTE_AP_READY : D30,
  LTE_PSM : D3,
  GPS_EN : D39,

  NRF_SCL : D13,
  NRF_SDA : D14,

  GPRS_RESET : D28, 
  GPRS_PWRKEY : D2,
  OPT_INT : D15,

  BAT_SET : D41,
  CHG_DET : D40,
  ADC_VBAT : D4,

};

var i2c = new I2C();
i2c.setup({sda:PINS.NRF_SDA, scl:PINS.NRF_SCL, bitrate:400000});

// SHTC3 humidity + temp - needs module
// LPS22HB pressure - we have a module for this

/// Returns a LIS3DH instance. callback when initialised. Then use 'read' to get data
exports.setAccelOn = function(isOn, callback) {
  if (this.LIS3DH) this.LIS3DH.off();
  delete this.LIS3DH;
  if (isOn) {
    if (callback) setTimeout(callback, 100, this.LIS3DH); // wait for first reading
    // {int:pin} isn't used yet, but at some point the module might include support
    return this.LIS3DH = require("LIS3DH").connectI2C(i2c, { int : PINS.LIS3DH_INT1});  
  }
};//tested ok

/// Returns a OPT3001 instance. callback when initialised. Then use 'read' to get data
exports.setOptoOn = function(isOn, callback) {
  if (this.OPT3001) this.OPT3001.off();
  delete this.OPT3001;
  if (isOn) {
    if (callback) setTimeout(callback, 1000, this.OPT3001); // wait for first reading
    // {int:pin} isn't used yet, but at some point the module might include support
    return this.OPT3001 = require("OPT3001").connectI2C(i2c, { int : PINS.OPT_INT });
  }
};//tested ok

// Turn cell connectivity on - will take around 8 seconds. Calls the `callback(usart)` when done. You then need to connect either SMS or QuectelM35 to the serial device `usart`
exports.setCellOn = function(isOn, callback) {
  if (isOn) {
    if (this.cellOn) {
      setTimeout(callback,10,Serial1);
      return;
    }
    var that=this;
    return new Promise(function(resolve) {
      Serial1.removeAllListeners();
      Serial1.on('data', function(x) {}); // suck up any data that gets transmitted from the modem as it boots (RDY, etc)
      Serial1.setup(115200,{tx:PINS.LTE_TXD, rx:PINS.LTE_RXD, cts:PINS.LTE_RTS});
      PINS.GPRS_PWRKEY.reset();
      setTimeout(resolve,200);
    }).then(function() {
      PINS.GPRS_PWRKEY.set();
      return new Promise(function(resolve){setTimeout(resolve,5000);});
    }).then(function() {
      this.cellOn = true;
      Serial1.removeAllListeners();      
      if (callback) setTimeout(callback,10,Serial1);
    });
  } else {
    this.cellOn = false;
    PINS.GPRS_PWRKEY.reset(); // turn power off.
    setTimeout(function() {
      PINS.GPRS_PWRKEY.set();
      if (callback) setTimeout(callback,1000);
    }, 800);
  }
};

/// Set whether the TP4054 should charge the battery (default is yes)
exports.setCharging = function(isCharging) {
  PINS.BAT_SET.write(!isCharging);
};

/// Set whether the TP4054 should charge the battery (default is yes)
exports.isCharging = function(isCharging) {
  return !PINS.CHG_DET.read();
};//tested ok

// Return GPS instance. callback is called whenever data is available!
exports.setGPSOn = function(isOn, callback) {
  if (!isOn) this.setCellOn(false,callback);
  else this.setCellOn(isOn, function(usart) {
    var at = require("AT").connect(usart);
    var gps = { at:at,on:function(callback) {
      callback=callback||function(){};
      at.cmd("AT+QGPS=1\r\n",1000,function cb(d) { // speed-optimal
        if (d&&d.startsWith("AT+")) return cb; // echo
        callback(d=="OK"?null:d);
      });
    },off:function(callback) {
      callback=callback||function(){};
      at.cmd("AT+QGPSEND\r\n",1000,function cb(d) {
        if (d&&d.startsWith("AT+")) return cb; // echo
        callback(d=="OK"?null:d);
      });
    },get:function(callback) {
      // ERROR: 516 means 'no fix'
      callback=callback||function(){};
      at.cmd("AT+QGPSLOC=2\r\n",1000,function cb(d) {
        if (!d) { callback({error:"Timeout"}); return; }
        if (d.startsWith("AT+")) return cb; // echo
        if (d.startsWith("+CME ERROR:")) callback({error:d.substr(5)});
        else if (d.startsWith("+QGPSLOC:")) {
          //+QGPSLOC: <UTC>,<latitude>,<longitude>,<hdop>,<altitude>,<fix>,<cog>,<spkm>,<spkn>,<date>,<nsat>
          d = d.substr(9).trim();
          var a = d.split(",");
          callback({
            raw : d,
          UTC:a[0],lat:+a[1],lon:+a[2],alt:+a[4]
        });
       } else callback({error:d});
      });
    }};
    gps.on(function(err) {
      callback(err, err?undefined:gps);
    });
  });
};
exports.PINS = PINS;
exports.AIN = D5;
exports.NRF_IO1 = D19;
exports.NRF_IO2 = D20;
exports.NRF_IO3 = D34;
exports.NRF_IO4 = D33;


/*
var i = require("iTracker");
var e = i.setAccelOn(true, function() {
  console.log(e.read()); // {x,y,z}
});

o=i.setOptoOn(true, function() {  
:  console.log(o.read());
:});

*/

