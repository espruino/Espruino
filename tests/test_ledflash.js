// From http://www.espruino.com/Flashing+Lights

Pin.prototype.startFlashing = function(period) {
  if (Pin.intervals==undefined) Pin.intervals = [];
  if (Pin.intervals[this]) clearInterval(Pin.intervals[this]);
  var on = false;
  var pin = this;
  Pin.intervals[this] = setInterval(function() {
    on = !on;
    digitalWrite(pin, on);
    result = 1;
  }, period);
}

D0.startFlashing(10);
setTimeout("clearInterval()",100);
