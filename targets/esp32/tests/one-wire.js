pinMode(D4,'input');
var ow = new OneWire(D4);

var sensors = ow.search().map(function (device) {
  return require("DS18B20").connect(ow, device);
});
console.log(sensors);