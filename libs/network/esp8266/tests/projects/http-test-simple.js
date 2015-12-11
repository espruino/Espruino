// Espruino esp8266 Socket tester - Copyright 2015 by Thorsten von Eicken
// HTTP client test from tests/test_http_*.js

test_host = "h.voneicken.com";
test_port = 4567;

var result = 0;
var http = require("http");

http.get("http://"+test_host+":"+test_port+"/ping", function(res) {
  console.log("Got response: " + JSON.stringify(res));
  res.on('data', function(data) {
    console.log(">" + data +"<");
    result = data=="pong\nEND";
    if (result) console.log("*** TEST SUCCESSFUL");
	});
});//.on('error', function(e) {
//  console.log("Got error: " + e.message);
//});*/
