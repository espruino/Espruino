clearWatch();
clearInterval();
// Spinner
SPI1.setup({baud:3200000, mosi:B5});
var FRONT_BUTTON = A2;

var hours = 3,mins = 20,secs;
var slowdown;
var speed;
var running;
var mode = 0;
var timePressed;
var pos=0;

// button press
setWatch(function(e) {
  if (e.time < timePressed+0.01) return; // skip button bounces
  timePressed = e.time;
  clearInterval();
  if (mode == 0) { // clock
    setInterval(function() {
      secs++;
      if (secs>59) {
       secs = 0;
       mins++;
       if (mins>59) {
         mins = 0;
         hours++;
         if (hours>11) {
          hours = 0;
         }
       }
      }
      var leds = new Uint8Array(12*3);
      var secled = parseInt(secs/5);
      leds[1+(11-secled)*3] = 255; // green
      var minled = parseInt(mins/5);
      leds[2+(11-minled)*3] = 255; // blue
      leds[0+(11-hours)*3] = 255; // red
      SPI1.send4bit(leds, 0b0001, 0b0011);
    }, 1000);
  } else if (mode == 1) { // spin
    speed = 20;
    slowdown = 1.1 + Math.random()*0.1;
    running = true;
    setInterval(function() {
      if (!running) {
        speed = speed * slowdown;
        changeInterval(0,speed);
        if (speed > 500) clearInterval();
      }
      pos++;
      if (pos>11) {
       pos = 0;
      }
      var leds = new Uint8Array(12*3);
      leds[0+(11-pos)*3] = 255; // red
      leds[2+(11-pos)*3] = 255; // blue
      SPI1.send4bit(leds, 0b0001, 0b0011);
    }, speed);
  } else if (mode == 2) { // random flick between 6
    speed = 20;
    slowdown = 1.1 + Math.random()*0.1;
    running = true;
    setInterval(function () {
      if (!running) {
        speed = speed * slowdown;
        changeInterval(0,speed);
        if (speed > 500) clearInterval();
      }
      pos = parseInt(Math.random()*6)*2;
      var leds = new Uint8Array(12*3);
      var r = 1+parseInt(Math.random()*6);
      leds[0+(11-pos)*3] = (r&1)?0:255; // red
      leds[1+(11-pos)*3] = (r&2)?0:255; // green
      leds[2+(11-pos)*3] = (r&4)?0:255; // blue
      SPI1.send4bit(leds, 0b0001, 0b0011);
    }, speed);
  } else if (mode == 3) {
    speed = 20;
    slowdown = 1.2 + Math.random()*0.1;
    running = true;
    setInterval(function () {
      if (!running) {
        speed = speed * slowdown;
        changeInterval(0,speed);
        if (speed > 500) clearInterval();
      }
      var patterns = [
        [0],
        [0,5],
        [0,4,8],
        [0,3,6,9],
        [0,2,4,6,9],
        [0,2,4,6,8,10],
      ];
      var r = parseInt(Math.random()*patterns.length);
      var leds = new Uint8Array(12*3);
      for (i in patterns[r]) {
        leds[1+patterns[r][i]*3] = 255; // green
        leds[2+patterns[r][i]*3] = 255; // blue
      }
      SPI1.send4bit(leds, 0b0001, 0b0011);
    }, speed);
  }
}, FRONT_BUTTON, { repeat: true, edge: "rising" });

// button release
setWatch(function(e) {
  if (e.time < timePressed+0.01) return; // skip button bounces
  if (e.time > timePressed+1) { // long press
    clearInterval();
    // go to next mode
    mode++;
    if (mode>3) mode=0;
    print(mode);
    // all LEDs off
    SPI1.send4bit(new Uint8Array(12*3), 0b0001, 0b0011);
  } else {
    // short press - signal for animation to slow down and stop
    running = false;
  }
  timePressed = e.time;
}, FRONT_BUTTON, { repeat: true, edge: "falling" });
function onInit() {
  // pull the front button down, so we don't need an external resistor
  pinMode(FRONT_BUTTON, "input_pulldown");
}
onInit();


// SIMPLE SPIN
SPI1.setup({baud:3200000, mosi:B5});
var FRONT_BUTTON = A2;

var slowdown;
var speed;
var running;
var timePressed;
var pos=0;

// button press
setWatch(function(e) {
  if (e.time < timePressed+0.01) return; // skip button bounces
  timePressed = e.time;
  // remove any animation that may have been happening
  clearInterval();
  // set up initial values
  speed = 20;
  slowdown = 1.1 + Math.random()*0.1;
  running = true;
  // start animation...
  setInterval(function() {
    if (!running) { // if the button was released...
      speed = speed * slowdown; // slow down
      changeInterval(0,speed); // use this to slow the timer
      if (speed > 500) clearInterval(); // if it's really slow then stop
    }
    pos++; // spin around
    if (pos>11) pos = 0; // wrap around when we get to the least LED
    // Now work out what pattern to show - just light up one light (with red and blue LEDs)
    var leds = new Uint8Array(12*3);
    leds[0+(11-pos)*3] = 255; // red
    leds[2+(11-pos)*3] = 255; // blue
    SPI1.send4bit(leds, 0b0001, 0b0011); // send to the lights
  }, speed); // speed for setInterval
}, FRONT_BUTTON, { repeat: true, edge: "rising" });

// button release
setWatch(function(e) {
  timePressed = e.time;
  // signal the animation to slow down and stop
  running = false;
}, FRONT_BUTTON, { repeat: true, edge: "falling" });

function onInit() {
  // pull the front button down, so we don't need an external resistor
  pinMode(FRONT_BUTTON, "input_pulldown");
}
onInit();






