fs=require("fs");

E.flashFatFS({format:true});

fs.readdirSync();
require("fs").writeFileSync("hello world.txt", "This is the way the world ends\nHello World\nnot with a bang but a whimper.\n");

fs.readdirSync();

// Fetch the DS18B20 module and store locally
fs.mkdir("node_modules");

var http = require("http");
http.get("http://www.espruino.com/modules/DS18B20.min.js", function(res) {
var contents = "";
  res.on('data', function(data) { contents += data; });
  res.on('close', function() { console.log(contents);
         fs.writeFileSync("node_modules/DS18B20.js", contents );});
  res.on('data', function(data) {
    
  });
});

/*
// paste to left handside of IDE - module will be loaded from FS

var ow = new OneWire(D23);

var sensors = ow.search().map(function (device) {
  return require("DS18B20").connect(ow, device);
});
console.log(sensors);
sensors[0].getTemp();
sensors[0].getTemp();
*/
