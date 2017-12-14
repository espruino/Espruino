// Connect to an access point and get a web page.
//
var ssid="guest";
var password="kolbanguest";

var wifi=require("Wifi");
wifi.connect(ssid, {password: password}, function() {
  console.log("Connected to access point, getting web page");
  var http = require("http");
  http.get("https://httpbin.org/ip", function(res) {
    res.on('data', function(data) {
      console.log(data);
    });
  });
});