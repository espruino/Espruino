
var toggle = true;

function updateLed() {
  digitalWrite(led, toggle);
  toggle=!toggle;
}
function button_down() {
  updateLed();
  print('button');
}
var red = D18;
var green = D19;
var blue = D22;
var led = blue;

digitalWrite(red, 0);
digitalWrite(green, 0);
digitalWrite(blue, 0);


setInterval(function () {updateLed();}, 1000);
setWatch(button_down, "D0", { repeat:true, edge:'rising' });

digitalWrite(red, 0);
digitalWrite(green, 0);
digitalWrite(blue, 0);

