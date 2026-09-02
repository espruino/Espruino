// espruino epaper badge JS code
/*

Badge.getTemperature().then(print);
Badge.getAccel().then(print);
Badge.setLEDs("#f00");

Badge.showTestScreen();
Badge.showImageFile("img.raw");
Badge.showRendering(function(g) { // Colors are: 0=black, 1=white, 2=yellow, 3=red
  g.clear().setColor(1).setFontVector(300).setFontAlign(0,0).drawString("Hello",400,240);
});
*/
(function(){
global.LED_EN = D20;
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
  var g = Graphics.createArrayBuffer(1,1,24,{color_order:"brg"}); // use 24 bit GFX to convert color types/strings
  var col = g.toColor(r,g,b);
  if (col==0) return LED_EN.reset(); // LEDs off
  LED_EN.set(); // LEDs on
  let arr = new Uint24Array(Badge.led_rgb.buffer);
  arr.fill(col);
  require("neopixel").write(D21,Badge.led_rgb);
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

/* Return a 400x240 2bpp graphics instance, which you can call `.flip()` on to update the screen.
   0=black, 1=white, 2=yellow, 3=red */
Badge.getGraphics = function() {
  var g = Graphics.createArrayBuffer(400,240,2); /* 0=black, 1=white, 2=yellow, 3=red */

  g.flip = function() {
    if (Badge.epaperBusy) throw new Error("ePaper is busy");
    Badge.epaperBusy = true;
    return epdInit().then(() => {
      eC(0x10);

      var line = new Uint16Array(100);
      /*var lut = new Uint16Array(256);
      for (var i=0;i<256;i++) lut[i] = (((i&0x03)<<8) | ((i&0x0C)<<10) | ((i&0x30)>>4) | ((i&0xC0)>>2)) * 0b0101;
      print(btoa(lut.buffer))*/
      var lut = new Uint16Array(E.toArrayBuffer(atob("AAAABQAKAA8AUABVAFoAXwCgAKUAqgCvAPAA9QD6AP8FAAUFBQoFDwVQBVUFWgVfBaAFpQWqBa8F8AX1BfoF/woACgUKCgoPClAKVQpaCl8KoAqlCqoKrwrwCvUK+gr/DwAPBQ8KDw8PUA9VD1oPXw+gD6UPqg+vD/AP9Q/6D/9QAFAFUApQD1BQUFVQWlBfUKBQpVCqUK9Q8FD1UPpQ/1UAVQVVClUPVVBVVVVaVV9VoFWlVapVr1XwVfVV+lX/WgBaBVoKWg9aUFpVWlpaX1qgWqVaqlqvWvBa9Vr6Wv9fAF8FXwpfD19QX1VfWl9fX6BfpV+qX69f8F/1X/pf/6AAoAWgCqAPoFCgVaBaoF+goKCloKqgr6DwoPWg+qD/pQClBaUKpQ+lUKVVpVqlX6WgpaWlqqWvpfCl9aX6pf+qAKoFqgqqD6pQqlWqWqpfqqCqpaqqqq+q8Kr1qvqq/68ArwWvCq8Pr1CvVa9ar1+voK+lr6qvr6/wr/Wv+q//8ADwBfAK8A/wUPBV8FrwX/Cg8KXwqvCv8PDw9fD68P/1APUF9Qr1D/VQ9VX1WvVf9aD1pfWq9a/18PX19fr1//oA+gX6CvoP+lD6Vfpa+l/6oPql+qr6r/rw+vX6+vr//wD/Bf8K/w//UP9V/1r/X/+g/6X/qv+v//D/9f/6//8=")));
      for (var y=0;y<240;y++) {
        E.mapInPlace(new Uint8Array(g.buffer, y*100, 100), line, lut);
        eD(line.buffer);
        eD(line.buffer);
      }
      return epdUpdate();
    }).then(epdSleep).then(() => {
      Badge.epaperBusy = false;
    });
  };
  return g;
}

Badge.showTestScreen = function() {
  return Badge.showRendering(function(g) {
    g.setColor(1).drawRect(0,0,399,239).drawRect(1,1,398,238);
    g.setBgColor(1).clear();
    for (var i=0;i<4;i++) g.setColor(i).fillRect(i*40,0,(i+1)*40,479);
    g.setColor(0);
    for (var i=0;i<480;i+=10) g.drawLine(799,i, 799-i,479);
    g.setColor(0).setFontVector(80).setFontAlign(0,0).drawString("Hello World",400,240);
    var env = process.env, mem=process.memory();
    g.setFont("6x8:2").setFontAlign(0,0).drawString(`Espruino ${env.VERSION} (${env.GIT_COMMIT})
https://espruino.com/Badge
${env.BOARD}
${mem.free} / ${mem.total} vars free
`,400,320);
  });
};

/* Call this with a string or an Error object to display an error String */
Badge.showError = function(error) {
  require("Storage").write("showing", "error");
  let stack = ((error instanceof Object) && error.stack) ? error.stack : undefined;
  let msg = (error instanceof Object) ? error.message : error.toString();
  return Badge.showRendering(function(g) {
    g.setBgColor(3).clearRect(0,0,799,479);
    g.setBgColor(1).clearRect(20,20,779,459);
    g.setColor(3).setFontVector(80).setFontAlign(0,0).drawString("ERROR",400,120);
    g.setColor(0).setFont("6x8:3").setFontAlign(0,-1).drawString(msg,400,196);
    if (stack) g.setFont("6x8:2").setFontAlign(-1,-1).drawString(stack, 30, 240);
    var env = process.env, mem=process.memory();
    g.setFont("6x8:2").setFontAlign(-1,-1).drawString(`Espruino ${env.VERSION} (${env.GIT_COMMIT})  ${env.BOARD}
https://espruino.com/Badge  ${mem.free} / ${mem.total} vars free
`,30,30);
  });
};

/* Show full-res 800x480 graphics on the display. We don't have a full-size
buffer for this, so we have a callback which is called to render each
slice in turn. Just render as-normal in gfxCallback.
0=black, 1=white, 2=yellow, 3=red */
Badge.showRendering = function(gfxCallback) {
  if (Badge.epaperBusy) throw new Error("ePaper is busy");
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

/* Load a raw 800x480x2 image file from storage.
0=black, 1=white, 2=yellow, 3=red */
Badge.showImageFile = function(filename) {
  if (Badge.epaperBusy) throw new Error("ePaper is busy");
  Badge.epaperBusy = true;
  return epdInit().then(() => {
    eC(0x10);
    eD(require("Storage").read(filename));
    return epdUpdate();
  }).then(epdSleep).then(() => {
    Badge.epaperBusy = false;
  });
};

// Puts the badge to sleep, waiting to restart on a button press. The button can be read with ESP32.getWakeupPin()
Badge.sleep = function() {
  LED_EN.reset(); // LEDs off
  ESP32.deepSleepExt1([BTN1,BTN2],0); // wait for buttons
};

/// Connect to wifi using details in wifi.json ({"ssid":"--","option":{"password":"---"}}). Returns a promise which only completes on success (on failure an error screen is displayed)
Badge.connectWiFi = function() {
  global.WIFI_INFO=require("Storage").readJSON("wifi.json",1)||{};
  if (!WIFI_INFO.ssid) {
    Badge.setLEDs("#101");
    Badge.showError("No WiFi Details\n\nConnect via Bluetooth/USB\nand write wifi.json");
    return new Promise(resolve => {}); // return a promise that never resolves
  }
  return new Promise(resolve => {
    require("Wifi").connect(WIFI_INFO.ssid, WIFI_INFO.options, function(err) {
      if (err) {
        console.log("WiFi error: "+err);
        Badge.showError("WiFi error: "+err).then(() => {
          Badge.sleep();
        });
        return;
      }
      console.log("WiFi Connected");
      resolve();
    });
  });
};

})();
