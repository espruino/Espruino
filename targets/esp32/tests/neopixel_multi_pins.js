// ESP32 neopixel test on 8 stripes with each 600 pixel

var Pins = [D18,D19,D17,D16,D26,D25,D33,D32];
Pins.forEach((e) => pinMode(e,"output"));


var np = require('neopixel');

var OFF     = 0;
var RED     = 40;
var BLUE    = 40<<8;
var GREEN   = 40<<16;
var CYAN    = (40<<8) | (40<<16);
var ORANGE  = 40 | (10<<16);
var PURPLE  = 10 | (40<<8);
var PINK    = 40 | (8<<8) | (6<<16);
var YELLOW  = 20 | (40<<16);

var Colors = [OFF, RED, BLUE, GREEN, PURPLE, YELLOW, CYAN, PINK, ORANGE];
var Names = ["OFF", "RED", "BLUE", "GREEN", "PURPLE", "YELLOW", "CYAN", "PINK", "ORANGE"];


var colorIndex = 0;
var pixel = 600;

var pixelBuffer = new Uint24Array(pixel);

npOff = function(){
  clearInterval();
  pixelBuffer.fill(OFF);
  Pins.forEach((pin) => {
    np.write(pin, pixelBuffer.buffer);
  });
};

npSet= function(pin,val,count){
  pixelBuffer.fill(val);
  np.write(pin,pixelBuffer.buffer);
};

simStart = function(pinsArray, colors, names, pixelCount) {
  var pinIndex = 0;
  // Start at index 1 to skip OFF (0) for the active phase
  var colorIndex = 1; 

  function nextStep() {
    if (pinIndex >= pinsArray.length) {
      console.log("=== Simulation starts over ===");
      pinIndex = 0;
    }

    var currentPin = pinsArray[pinIndex];
    var activeColor = colors[colorIndex];
    var colorName = names[colorIndex];

    console.log("Switch Pin ON: " + currentPin + " -> LEDs: " + colorName);

    // 1. Turn current pin's NeoPixels to its dedicated color
    npSet(currentPin, activeColor, pixelCount);

    setTimeout(function() {
      console.log("Switch Pin OFF: " + currentPin + " -> LEDs: OFF");

      // 2. Turn current pin's NeoPixels OFF
      npSet(currentPin, colors[0], pixelCount);

      // 3. Move to the next pin and shift to the next color for that pin
      pinIndex++;

      colorIndex++;
      if (colorIndex >= colors.length) {
        colorIndex = 1; // Reset back to the first active color (skip OFF)
      }

      setTimeout(nextStep, 1000);
    }, 1000);
  }
  console.log("=== Starting Pin and NeoPixel Simulation ===");
  nextStep();
};

stopSim = function() {
  console.log("=== Stopping simulation and turning everything off ===");
  clearInterval();
  npOff();
};

  
// Starts the simulation with your parameters
setTimeout(() => simStart(Pins, Colors, Names, pixel), 1000);
