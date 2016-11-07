// Start being an access point.
// Start being a web server.
// When a request arrives from a browser, build a page which contains the
// list of access points to which we can connect.
// When the user selects an access point with password, connect to that access point.

// 1. Be an access point
var wifi = require("Wifi");
wifi.startAP("MYESP32", {
  "authMode": "open"
}, function(err) {
  console.log("AP now started: " + err);
});