
/* Copyright (c) 2018 Gordon Williams, Pur3 Ltd. See the file LICENSE for copying permission. */
/* 
Nordic Thingy Support Library

TODO:

Beep
MPU9250 support - the library at the moment only works for the magnetometer
Microphone support - will most likely need native support

*/


var MPU_PWR_CTRL = V8;
var MPU_INT = D6;
var MIC_PWR_CTRL = V9;
var MIC_DOUT = D25;
var MIC_CLK = D26;
var CCS_PWR_CTRL = V10;
var CCS_RST = V11;
var CCS_WAKE = V12;
var CCS_INT = D22;
var LIS_INT = D12;
var SENSE_LEDR = V13;
var SENSE_LEDG = V14;
var SENSE_LEDB = V15;
var SENSE_LEDS = [SENSE_LEDR,SENSE_LEDG,SENSE_LEDB];
var LPS_INT = D23;
var HTS_INT = D24;
var BH_INT = D31;
var BATTERY_CHARGE = D17;
var BATTERY_VOLTAGE = D28;
var BATTERY_MONITOR = V4;
var SPEAKER	= D27;
var SPK_PWR_CTRL= D29;

var i2c = new I2C();
i2c.setup({sda:7,scl:8,bitrate:400000});
exports.I2C = i2c;
var i2ce = new I2C();
i2ce.setup({sda:14,scl:15,bitrate:400000});
exports.I2CE = i2ce;

// ------------------------------------------------------------------------------------------- LIS2DH12
// Get repeated callbacks with {x,y,z}. Call with no argument to disable
exports.onAcceleration = function(callback) {
  if (callback) {
    if (!this.accel) this.accel = require("LIS2DH12").connectI2C(i2ce/*, { int:LIS_INT } - not used */);
    this.accel.callback = callback;
    this.accel.setPowerMode("low");
  } else {
    if (this.accel) this.accel.setPowerMode("powerdown");
    this.accel = undefined;
  }
};
// Get one callback with a new acceleration value
exports.getAcceleration = function(callback) {
  if (!this.accel) {
    require("LIS2DH12").connectI2C(i2ce/*, { int:LIS_INT } - not used */).readXYZ(callback);
  } else {
    this.accel.readXYZ(callback);
  }
}

// ------------------------------------------------------------------------------------------- LPS22HB
// Get repeated callbacks with {pressure,temperature}. Call with no argument to disable
exports.onPressure = function(callback) {
  if (callback) {
    if (!this.pressure) {
      this.pressure = require("LPS22HB").connectI2C(i2c, {int:LPS_INT});
      this.pressure.on('data',function(d) { this.pressureCallback(d); }.bind(this));
    }
    this.pressureCallback = callback;
  } else {
    if (this.pressure) this.pressure.stop();
    this.pressure = undefined;
    this.pressureCallback = undefined;
  }
};
// Get one callback with a new {pressure,temperature} value
exports.getPressure = function(callback) {
  if (!this.pressure) {
    var p = require("LPS22HB").connectI2C(i2c, {int:LPS_INT});
    p.read(function(d) {
      p.stop();
      callback(d);
    });
  } else {
    this.pressure.read(callback);
  }
}
// ------------------------------------------------------------------------------------------- HTS221
// Get repeated callbacks with {humidity,temperature}. Call with no argument to disable
exports.onHumidity = function(callback) {
  if (callback) {
    if (!this.humidity) {
      this.humidity = require("HTS221").connect(i2c, {int:HTS_INT});
      this.humidity.on('data',function(d) { this.humidityCallback(d); }.bind(this));
    }
    this.humidityCallback = callback;
  } else {
    if (this.humidity) this.humidity.stop();
    this.humidity = undefined;
    this.humidityCallback = undefined;
  }
};
// Get one callback with a new {humidity,temperature} value
exports.getHumidity = function(callback) {
  if (!this.humidity) {
    this.onHumidity(function(d) {
      this.onHumidity();
      callback(d);
    }.bind(this));
  } else {
    this.humidity.read(callback);
  }
}
// ------------------------------------------------------------------------------------------- HTS221
// Get repeated callbacks with air quality `{eC02,TVOC}`. Call with no argument to disable
exports.onGas = function(callback) {  
  if (callback) {
    if (!this.gas) {
      CCS_PWR_CTRL.set(); // CCS on
      CCS_RST.set(); // no reset
      CCS_WAKE.reset(); // wake
      this.gas = require("CCS811").connectI2C(i2c, { int : CCS_INT });
      this.gas.on('data',function(d) { this.gasCallback(d); }.bind(this));
    }
    this.gasCallback = callback;
  } else {
    if (this.gas) {
      this.gas.stop();
      CCS_RST.reset(); // reset (so it doesn't power it via RST pin)
      CCS_PWR_CTRL.reset(); // CCS off
    }
    this.gas = undefined;
    this.gasCallback = undefined;
  }
};
/* Get one callback with a new air quality value `{eC02,TVOC}`. This may not be useful
as the sensor takes a while to warm up and produce useful values */
exports.getGas = function(callback) {
  if (!this.gas) {
    this.onGas(function(d) {      
      this.onGas();
      callback(d);
    }.bind(this));
  } else {
    callback(this.gas.get());
  }
}
// ------------------------------------------------------------------------------------------- HTS221
// Get repeated callbacks with color `{r,g,b,c}`. Call with no argument to disable
exports.onColor = function(callback) {  
  if (callback) {
    if (!this.color) {
      digitalWrite(SENSE_LEDS,0); // all LEDs on
      this.color = require("BH1745").connectI2C(i2c/*, {int : BH_INT}*/);
      this.colorInt = setInterval(function() {
        this.colorCallback(this.color.read());
      }.bind(this), 200);
    }
    this.colorCallback = callback;
  } else {
    if (this.color) {
      clearInterval(this.colorInt);
      this.color.stop();
      digitalWrite(SENSE_LEDS,7); // all LEDs off
    }
    this.color = undefined;
    this.colorInt = undefined;
    this.colorCallback = undefined;
  }
};
// Get one callback with a new color value `{r,g,b,c}`
exports.getColor = function(callback) {
  if (!this.color) {
    this.onColor(function(d) {      
      this.onColor();
      callback(d);
    }.bind(this));
  } else {
    callback(this.color.read());
  }
}
// ------------------------------------------------------------------------------------------- Battery
// Returns the state of the battery (immediately, or via callback) as { charging : bool, voltage : number }
exports.getBattery = function(callback) {
  BATTERY_MONITOR.set();
  var result = { 
    charging : BATTERY_CHARGE.read(),
    voltage : E.getAnalogVRef()*analogRead(BATTERY_VOLTAGE)*1500/180
  };
  BATTERY_MONITOR.reset();
  if (callback) callback(result);
  return result;
}
// ------------------------------------------------------------------------------------------- Speaker
// Play a sound, supply a string/uint8array/arraybuffer, samples per second, and a callback to use when done
// This can play up to 3 sounds at a time (assuming ~4000 samples per second)
exports.sound = function(waveform, pitch, callback) {  
  if (!this.sounds) this.sounds=0;
  if (this.sounds>2) throw new Error("Too many sounds playing at once");
  var w = new Waveform(waveform.length);
  w.buffer.set(waveform);
  w.on("finish", function(buf) {
    this.sounds--;
    if (!this.sounds) {
      SPK_PWR_CTRL.reset();
      digitalWrite(SPEAKER,0);
    }
    if (callback) callback();
  }.bind(this));
  if (!this.sounds) {
    analogWrite(SPEAKER, 0.5, {freq:40000});
    SPK_PWR_CTRL.set();
  }
  this.sounds++;
  w.startOutput(SPEAKER, pitch);
};
// Make a simple beep noise. frequency in Hz, length in milliseconds. Both are optional.
exports.beep = function(freq, length) {
  length = (length>0)?length:250;
  freq = (freq>0)?freq:500;
  analogWrite(SPEAKER, 0.5, {freq:freq});
  SPK_PWR_CTRL.set();
  if (this.beepTimeout) clearTimeout(this.beepTimeout);
  this.beepTimeout = setTimeout(function() {
    delete this.beepTimeout;
    SPK_PWR_CTRL.reset();
    digitalWrite(SPEAKER,0);
  }.bind(this), length);
};
// Record audio for the given number of samples, at 8192kHz 8 bit.
// This can then be fed into Thingy.sound(waveform, 8192). RAM is scarce, so realistically 1 sec is a maximum.
exports.record = function(samples, callback) {
  var gain = 0x48; // gain ( 0 -> 0x50, 0x28 default )
  var buf = new ArrayBuffer(2049); // 2x 1k byte sample buffers for DMA (+1 for byte shift)
  var bufAddr = E.getAddressOf(buf,true);
  if (!bufAddr) throw new Error("Unable to create a buffer");
  var result = new Uint8Array(samples);
  var resultIdx = 0;
  var p = 0; // 1 or 0 (which buffer)
  MIC_PWR_CTRL.set();
  MIC_CLK.mode("output");
  MIC_DOUT.mode("input");  
  poke32(0x4001D504,0x08400000); // 1.032MHz clock default
  poke32(0x4001D508,1); // mono, left on falling edge
  poke32(0x4001D518,gain); // gain left
  poke32(0x4001D51C,gain); // gain right
  poke32(0x4001D540,MIC_CLK.getInfo().num); // CLK
  poke32(0x4001D544,MIC_DOUT.getInfo().num); // DIN
  poke32(0x4001D560,bufAddr); // PTR
  poke32(0x4001D564,512); // MAXCNT
  poke32(0x4001D500,1); // enable PDM
  poke8(0x4001D100,0); // event start
  poke8(0x4001D104,0); // event stop
  poke8(0x4001D108,0); // event end
  poke8(0x4001D000,1); // START TASK
  poke32(0x4001D560,bufAddr+1024); // second pointer
  // 16kHz output 16 bit, 32kByte/sec = will be done in 30ms
  // Poll more often so we're more likely to catch it (no way to hook IRQs yet)
  var i = setInterval(function() {
    if (peek8(0x4001D108)) { // end event
      poke8(0x4001D108,0); // clear end
      poke32(0x4001D560,bufAddr+p*1024); // push new address in
      // quick copy, drop every other sample, 16->8 bits
      result.set(new Uint32Array(buf,1+(p*1024),256),resultIdx);
      // advance and stop if required
      p=1-p;
      resultIdx+=256;
      if (resultIdx>=result.length) stop();
    }
  },5);
  function stop() {
    clearInterval(i); // stop polling
    poke8(0x4001D004,1); // STOP TASK
    poke8(0x4001D500,0); // disable PDM
    poke32(0x4001D540,0xFFFFFFFF); // disconnect CLK
    poke32(0x4001D544,0xFFFFFFFF); // disconnect DIN    
    MIC_PWR_CTRL.reset(); // mic off
    // Now we have some time, add 128 to each item
    E.mapInPlace(result,result,function(x){return x+128;});
    // call the callback
    if (callback) setTimeout(callback,0,result);
  }
  return result;
};

// Reinitialise any hardware that might have been set up before being saved
E.on('init',function() {
  if (exports.accel && exports.accel.callback) { 
    var c = exports.accel.callback; 
    exports.accel = undefined; 
    exports.onAcceleration(c); 
  }
  if (exports.pressureCallback) { 
    exports.pressure = undefined; 
    exports.onPressure(exports.pressureCallback); 
  }
  if (exports.humidityCallback) { 
    exports.humidity = undefined; 
    exports.onHumidity(exports.humidityCallback);
  }
  if (exports.gasCallback) { 
    exports.gas = undefined; 
    exports.onGas(exports.gasCallback); 
  }
  if (exports.colorCallback) { 
    exports.color = undefined; 
    exports.onColor(exports.colorCallback); 
  }
});
