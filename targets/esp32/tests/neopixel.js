const neopixel = require("neopixel");
var  NP_PIN = D18;
  // 48 bytes RGB data
var buf = new Uint8ClampedArray(16*3);
var intervalId;

function stopAnimation(){
  if (intervalId) clearInterval(intervalId)
  buf.fill(0);
  neopixel.write(NP_PIN, buf);
  console.log("Stopped + All OFF");
}

// 4x4 NeoPixel pad = 16 LEDs
function startAnimation() {
  for(let i = 0; i < 16; i++) {
    // Random RGB (0-20 max brightness)
    buf[i*3 + 0] = Math.floor(Math.random() * 21);  // R
    buf[i*3 + 1] = Math.floor(Math.random() * 21);  // G  
    buf[i*3 + 2] = Math.floor(Math.random() * 21);  // B
  }
  console.log("4x4 pad random colors:", buf);
  neopixel.write(NP_PIN, buf);
}

// Store interval ID + run
intervalId = setInterval(startAnimation, 2000);