// Espruino esp8266 Socket tester - Copyright 2015 by Thorsten von Eicken

tve_ssid = "tve-home";
tve_pwd  = "tve@home";

// Test misc
var wifi = require("Wifi");
var esp8266 = require("ESP8266");
var async = require("Async");
var http = require("http");

// Overall test outcome
var ok=true;

var url;

// barf if false
function expect(what, outcome, value) {
  if (!outcome) {
    console.log("In " + what + " unexpected value:\n", value, "\nOooops!");
    ok = false;
  } else {
    console.log("OK " + what);
  }
}

function testCB(msg, next, timeout) {
  if (timeout === undefined) timeout = 1000;
  console.log("Wait for", msg, "in", timeout/1000.0, "sec");
  var n = 0;
  setTimeout(function() {
    if (n++ === 0) { console.log("OOPS: no callback for " + msg); ok = false; next(); }
  }, timeout);
  return function(x) { console.log("GOT CB for " + msg); if (n++ === 0) next(x); };
}


function testSock1() {
  async.sequence([
    // make a simple HTTP request
    function() {
      var self = this;
      var data = "";
      http.get(url+"/ping", function(res) {
        res.on('data', function(d) { data += d; });
        res.on('close', function() { self.next(data); });
      });
      console.log("Requesting ping...");
    },
    function(data) {
      expect("ping", data == "pong", data);
      this.next();
    }

    ],
    {},
    function() {
      console.log("=== test ended:", (ok?"SUCCESS":"ERROR"),"===");
    });
}

console.log("***** TEST START *****");
console.log(process.memory());

if (typeof test_host === 'undefined' || typeof test_port === 'undefined') {
  console.log("please set test_host, and test_port");
  console.log("then wifi.save();");
} else {
  url = "http://" + test_host + ":" + test_port;
  testSock();
  console.log(process.memory());
}

