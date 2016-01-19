// Espruino esp8266 Socket tester - Copyright 2015 by Thorsten von Eicken

// Test multiple concurrent connections

test_host = "h.voneicken.com";  // <======= configure for your local server running the test_http.rb sinatra server
test_port = 4567;
require("ESP8266").setLog(0);

var test_ip = 0; // will be resolved

// Test misc
var wifi = require("Wifi");
var esp8266 = require("ESP8266");
var async = require("Async");
var http = require("http");
//var url = require("url");

// Overall test outcome
var ok=true;

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

function doGet(host, path, next) {
  var len = 0;
  var data = "";
  var q = { host: host, port: test_port, path: path, method: 'GET' };
  http.get(q, function(res) {
    res.on('data', function(d) { len += d.length; data = d; });
    res.on('close', function() {
      // We expect all responses to end with "END"
      var l = data.length;
      expect("get END", l < 3 || data.substring(l-3, l) === "END", data);
      next(len);
    });
  });
}

function testSockMulti() {
  async.sequence([
    // make a simple HTTP request
    function() {
      doGet(test_host, "/ping", this.next);
    },
    function(len) {
      expect("ping", len === 8, len);
      this.next();
    },

    // make a few concurrent requests
    function() {
      wifi.getHostByName(test_host, this.next);
    },
    function(ip) {
      var cnt=3;
      var self=this;
      function done(len) {
        expect("ping", len === 8, len);
        if (--cnt === 0) self.next();
      }
      for (var i=cnt; i>0; i--) doGet(ip, "/ping", done);
    },

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
  urlPrefix = "http://" + test_host + ":" + test_port;
  testSockMulti();
  console.log(process.memory());
}

