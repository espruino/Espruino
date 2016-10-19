// Connect to an access point and be a simple web server
//
var ssid="guest";
var password="kolbanguest";

var wifi=require("Wifi");
wifi.connect(ssid, {password: password}, function() {
  console.log("Connected to access point, listening for clients on port 8080.");
  var http = require("http");
  http.createServer(function (req, res) {
    res.writeHead(200);
    res.end("Hello World"); 
  }).listen(8080);
});