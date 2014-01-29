Pin.prototype.moveTo = function (pos,time) {
  if (time==undefined) time = 1000;
  var pin = this;
  var t = 0;
  var int = setInterval(function() {
    digitalPulse(pin, 1, 1+E.clip(pos,0,1));
    t += 20;
    if (t>time) clearInterval(int);
  }, 20);
};


var VENT = B12;
var WATER = B13;

Pin.prototype.moveTo = function (pos,time) {
  if (time==undefined) time = 1000;
  var pin = this;
  var amt = 0;
  if (servoPos==undefined) servoPos = {};
  if (servoPos[pin]==undefined) servoPos[pin] = pos;

  var int = setInterval(function() {
    if (amt>1) {
      clearInterval(int);
      servoPos[pin] = pos;
      amt = 1;
    }
    digitalPulse(pin, 1, 1+E.clip(pos*amt + servoPos[pin]*(1-amt),0,1));
    amt += 1000.0 / (20*time);
  }, 20);
};
function ventClose() { VENT.moveTo(0.65, 2000); }
function ventOpen() { VENT.moveTo(0.1, 2000); }
function waterOn() { WATER.moveTo(0.9, 2000); }
function waterOff() { WATER.moveTo(0.2, 2000); }
