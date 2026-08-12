// espruino epaper badge JS code
/*

Badge.getTemperature().then(print);
Badge.getAccel().then(print);
Badge.setLEDs("#f00");

Badge.showTestImage();
Badge.showImageFile("img.raw");
Badge.showRendering(function(g) { // Colors are: 0=black, 1=white, 2=yellow, 3=red
  g.clear().setColor(1).setFontVector(300).setFontAlign(0,0).drawString("Hello",400,240);
});
*/
(function(){
global.LED_EN = D20; // inverted
var led_rgb = new Uint8Array(3*10);
require("neopixel").write(D21, led_rgb); // set all neopixels to off

var i2c = new I2C();
i2c.setup({sda:D5, scl:D6});

global.Badge = {
  i2c : i2c,
  led_rgb : led_rgb, // RGB buffer
  epaperBusy : false
};

//i2c.readReg(0x38, 0,1); // AHT20
//i2c.readReg(0x1E, 0,1); // KX022-1020
Badge.getTemperature = function() {
  const AHT20_ADDR = 0x38;
  return new Promise(resolve => {
    // 1. Initialize the sensor (Command 0xBE with params 0x08, 0x00)
    // This is required once on startup, but safe to run before a measurement.
    i2c.writeTo(AHT20_ADDR, [0xBE, 0x08, 0x00]);
    setTimeout(function() { // Wait for calibration/initialization to complete
      // 2. Trigger Measurement (Command 0xAC with params 0x33, 0x00)
      i2c.writeTo(AHT20_ADDR, [0xAC, 0x33, 0x00]);
      setTimeout(function() { // Crucial: Wait 80ms for the measurement to complete
        // 3. Read 6 bytes of data (using 0x71 as the status/read command)
        const data = i2c.readReg(AHT20_ADDR, 0x71, 6);
        // 4. Check if the sensor is busy (Bit 7 of the status byte)
        const isBusy = (data[0] & 0x80) !== 0;
        if (isBusy) {
          throw new Error("Sensor is busy, measurement not ready.");
        }
        // 5. Parse Humidity (20-bit value split across bytes 1, 2, and high nibble of 3)
        const rawHumidity = ((data[1] << 12) | (data[2] << 4) | (data[3] >> 4));
        const humidity = (rawHumidity / Math.pow(2, 20)) * 100;
        // 6. Parse Temperature (20-bit value split across low nibble of 3, 4, and 5)
        const rawTemperature = (((data[3] & 0x0F) << 16) | (data[4] << 8) | data[5]);
        const temperature = ((rawTemperature / Math.pow(2, 20)) * 200) - 50;

        resolve({
          temperature: temperature, // in °C
          humidity: humidity        // in % RH
        });
      }, 80);
    }, 10);
  });
}

Badge.getAccel = function() {
  let promise;
  const KX_ADDR = 0x1E;
  // CNTL1 register (0x18): Set PC1 bit (bit 7) to 1 to enable Active Mode.
  // We keep default settings: G-range of +/-2g, and 16-bit resolution.
  i2c.writeTo(KX_ADDR, [0x18, 0x80]);
  // Now return the result after a pause
  return new Promise(resolve => {
    setTimeout(function() {
      // Read 6 bytes starting from XOUT_L (0x06) through ZOUT_H (0x0B)
      const data = i2c.readReg(KX_ADDR, 0x06, 6);
      // set to inactive
      i2c.writeTo(KX_ADDR, [0x18, 0x00]);
      // return result
      resolve((new Int16Array(data.buffer)).slice().map(n => n/16384));
    }, 20);
  });
}

Badge.setLEDs = function(r,g,b) {
  var g = Graphics.createArrayBuffer(1,1,24,{color_order:"bgr"}); // use 24 bit GFX to convert color types/strings
  var col = g.toColor(r,g,b);
  if (col==0) return LED_EN.set(); // LEDs off
  LED_EN.reset();
  let arr = new Uint24Array(led_rgb.buffer);
  arr.fill(col);
  require("neopixel").write(D21, led_rgb);
}

Badge.scan = function() {
  var gatt, service, tx, rx;
  var text = "\x10print(JSON.stringify(ID))\n";
  // left-right bluetooth scan effect
  LED_EN.reset();
  let arr = new Uint24Array(led_rgb.buffer);
  arr.fill(10);
  require("neopixel").write(D21, led_rgb);
  let animInt = setInterval(function() {
    var n = 4.5+3*Math.sin(getTime()*2);
    for (var i=0;i<10;i++)
      arr[i] = E.HSBtoRGB(0.7,1,Math.pow(E.clip(1-0.2*Math.abs(i-n),0,1),2)*0.2);
    require("neopixel").write(D21, led_rgb);
  }, 50);

  let found = false;
  return new Promise(resolve => NRF.setScan(function(d) {
    if (found || d.rssi < -50) return;
    found = true;
    NRF.setScan();
    clearInterval(animInt);
    setLEDs(0x7F0000);
    print("Found ",d);
    resolve(d);
    animInt = setTimeout(function() {
      setLEDs();
    }, 1000);
  }, { filters: [{ namePrefix:"Espruino" }] })).then(function(device) {
    return device.gatt.connect();
  }).then(function(d) {
    gatt = d;
    return d.getPrimaryService("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
  }).then(function(s) {
    service = s;
    return service.getCharacteristic("6e400002-b5a3-f393-e0a9-e50e24dcca9e");
  }).then(function(c) {
    tx = c;
    return service.getCharacteristic("6e400003-b5a3-f393-e0a9-e50e24dcca9e");
  }).then(function(c) {
    rx = c;
    rx.on('characteristicvaluechanged', function(event) {
      var d = E.toString(event.target.value.buffer);
      print("RX", E.toJS(d));
    });
    return rx.startNotifications();
  }).then(function() {
    function sender(resolve, reject) {
      if (text.length) {
        tx.writeValue(text.substr(0,20)).then(function() {
          sender(resolve, reject);
        }).catch(reject);
        text = text.substr(20);
      } else  {
        resolve();
      }
    }
    return new Promise(sender);
  }).then(function() {
    return new Promise(resolve => setTimeout(resolve, 1000));
  }).then(function() {
    gatt.disconnect();
  });
}

// ePaper
const CS = D4, DC = D10, RST = D1, BUSY = D0; // BUSY keeps changing?
const Source_BITS  = 800;
const Gate_BITS   = 680;
const ALLSCREEN_BYTES =  96000;

CS.set();
DC.set();
RST.set();
BUSY.read();
/*var spi = new SPI();
spi.setup({ sck : D7, mosi: D8 });*/
var spi = SPI1;
spi.setup({ baud : 4000000, sck : D7, mosi: D8 });

// epaper: write command
function eC(command) {
	CS.reset();
	DC.reset();  // D/C#   0:command  1:data
	spi.write(command);
	CS.set();
}
// epaper: write data
function eD(datas) {
	CS.reset();
	DC.set();  // D/C#   0:command  1:data
	spi.write(datas);
	CS.set();
}
// hacky in-place delay (returns promise)
function delay_ms(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}
// wait for not busy (returns promise)
function epdWait() {
  return new Promise(resolve => {
    function poll() {
      if (BUSY.read()) resolve();
      else setTimeout(poll,1);
    }
    poll();
  });
}
// init epd - return a promise
function epdInit() {
	return delay_ms(20).then(() => {//At least 20ms delay
	  RST.reset();		// Module reset
	  return delay_ms(10); //At least 10ms delay
  }).then(() => {
	  RST.set();
	  return delay_ms(10); //At least 10ms delay
  }).then(() => epdWait() //waiting for the electronic paper IC to release the idle signal
  ).then(() => {
    eC(0x00);	//0x00
    eD(0x2B);
    eD(0x29);

    eC(0x06);	//0x06
    eD(0x0F);
    eD(0x8B);
    eD(0x93);
    eD(0xC1); //0xC1

    eC(0x50);	//0x50
    eD(0x37);

    eC(0x30);	//0x30
    eD(0x08);

    eC(0x61);//0x61
    eD(Source_BITS/256);
    eD(Source_BITS%256);
    eD(Gate_BITS/256);
    eD(Gate_BITS%256);

    eC(0x62);
    eD(0x76);
    eD(0x76);
    eD(0x76);
    eD(0x5A);
    eD(0x9D);
    eD(0x8A);
    eD(0x76);
    eD(0x62);

    eC(0x65);	//0x65
    eD(0x00);
    eD(0x00);
    eD(0x00);
    eD(0x00);

    eC(0xE0);	//0xE3
    eD(0x10);

    eC(0xE7);	//0xE7
    eD(0xA4);

    eC(0xE9);
    eD(0x01);

    //Fast
    eC(0xEF);
    eD(0x01);
    eC(0xF6);
    eD(0x20);

    eC(0xEF);
    eD(0x00);

    eC(0xE0);
    eD(0x12);

    eC(0xE6);
    eD(92);

    eC(0xA5);
    eD(0x00);
	  return epdWait();
  }).then(() => {
	  eC(0x04); //Power on
	  return epdWait();          //waiting for the electronic paper IC to release the idle signal
  });
}
// Sleep EPD - return promise
function epdSleep() {
  eC(0X02);   //power off
  eD(0x00);
  return epdWait().then(() => {          //waiting for the electronic paper IC to release the idle signal
    eC(0X07);   //deep sleep
    eD(0xA5);
  });
}
// Actually update what's on the display. Returns promise, takes ~12 sec
function epdUpdate(){
  eC(0x12); //Display Update Control
  eD(0x00);
  return epdWait();
}

Badge.showTestImage = function() {
  Badge.epaperBusy = true;
  return epdInit().then(() => {
    eC(0x10);
    var g = Graphics.createArrayBuffer(400,240,2); /* 0=black, 1=white, 2=yellow, 3=red */
    g.setColor(1).drawRect(0,0,399,239).drawRect(1,1,398,238);
    g.drawLine(0,0,300,300);
    for (var i=0;i<4;i++) g.setColor(i).fillRect(4+i*20,4,4+(i+1)*20,50);
    g.setColor(1).setFontVector(50).setFontAlign(0,0).drawString("Hello World",200,120);
    var line = new Uint16Array(100);
    var lut = new Uint16Array(256);
    for (var i=0;i<256;i++)
      lut[i] = (((i&0x03)<<8) | ((i&0x0C)<<10) | ((i&0x30)>>4) | ((i&0xC0)>>2)) * 0b0101;
    for (var y=0;y<240;y++) {
      E.mapInPlace(new Uint8Array(g.buffer, y*100, 100), line, lut);
      eD(line.buffer);
      eD(line.buffer);
    }
    return epdUpdate();
  }).then(epdSleep).then(() => {
    Badge.epaperBusy = false;
  });

}

/* Show full-res graphics on the display. We don't have a full-size
buffer for this, so we have a callback which is called to render each
slice in turn. Just render as-normal in gfxCallback */
Badge.showRendering = function(gfxCallback) {
 Badge.epaperBusy = true;
  return epdInit().then(() => {
    eC(0x10);
    var g = Graphics.createArrayBuffer(800,48,2); /* 0=black, 1=white, 2=yellow, 3=red */
    for (var y=0;y<480;y+=48) {
      g.clear(1).setOffset(0,-y).setColor(1);
      gfxCallback(g);
      eD(g.buffer);
    }
    return epdUpdate();
  }).then(epdSleep).then(() => {
    Badge.epaperBusy = false;
  });
};

Badge.showImageFile = function(filename) {
 Badge.epaperBusy = true;
  return epdInit().then(() => {
    eC(0x10);
    eD(require("Storage").read(filename).substr(0, 800*(480-48)>>2));
    return epdUpdate();
  }).then(epdSleep).then(() => {
    Badge.epaperBusy = false;
  });
};

Badge.sleep = function() {
  LED_EN.reset(); // LEDs off
  ESP32.deepSleepExt1([BTN1,BTN2],0); // wait for buttons
};
})();