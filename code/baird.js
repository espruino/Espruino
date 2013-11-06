var FAN_SPEED_IN = E3;
var FAN_SPEED_OUT = E1;
var LEDS = [D12,D13,D14,D15];
var IMG = (
"11111110"+
"10000010"+
"10000010"+
"10000010"+
"10000010"+
"10000010"+
"10000010"+
"11111110");

var pos = 0;
function onDigit(e) {
  digitalWrite(LEDS, (IMG[pos]=="1") ? 255 : 0);
  pos++;
}
var digitInterval = setInterval(onDigit, 10);

function onFanMovement(e) {
  var d = e.time-lastTime;
  lastTime = e.time;
  pos = 0;
  changeInterval(digitInterval, 1000*d/(7*8));
}
setWatch(onFanMovement, FAN_SPEED_IN, { repeat: true, edge: 'falling' });
digitalWrite(FAN_SPEED_OUT,0)

var IMG = (
"10000000"+
"10000000"+
"10000000"+
"10000000"+
"10000000"+
"10000000"+
"10000000"+
"10000000");

var IMG = (
"0000000"+
"0000000"+
"0000000"+
"1111000"+
"1001000"+
"1001000"+
"1001000"+
"1111000");

var IMG = (
"00010000"+
"00010000"+
"00010000"+
"11111111"+
"00010000"+
"00010000"+
"00010000"+
"00010000");

var IMG = (
"00011000"+
"00111100"+
"00011000"+
"00000000"+
"01100110"+
"00011000"+
"01100110"+
"00000000");