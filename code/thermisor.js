// Code to handle a 5k @ 25 degree C thermistor

function getTemp() {
  digitalWrite(D0,0);
  digitalWrite(D2,1);
  var val = analogRead(D1);
  var ohms = 5600*val/(1-val);
  var A = 0.00128463;
  var B = 0.00023625;
  var C = 0.000000092697;
  var W = Math.log(ohms);
  var temp = 1 / (A + W * (B+C * W*W)) - 273.15
  return temp;
}
