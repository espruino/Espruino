Pin.prototype.pwm = function(dutyCycle, speed) {
  var pin = this;
  if (Pin.intervals===undefined)
    Pin.intervals = {};
  if (Pin.intervals[pin]!==undefined)
    clearInterval(Pin.intervals[pin]);
  var pulseLen = dutyCycle*1000/speed;
  Pin.intervals[pin] = setInterval(function() {
   // digitalPulse(pin, 1, pulseLen);
  }, 1000/speed);
};

var p = new Pin();

pos = 0;
var s = function() {
  pos = pos + 0.1;
  p.pwm(Math.sin(pos)*0.5 + 0.5, 300); // 300Hz
  if (pos>1) {
    result=1;
    clearInterval(); // finish
  }
};
setInterval(s,10);
