// Espruino esp8266 Socket tester - Copyright 2015 by Thorsten von Eicken

test_host = "h.voneicken.com";
test_port = 4567;
require("ESP8266").setLog(2);

// Test misc
var wifi = require("Wifi");
var esp8266 = require("ESP8266");
var async = require("Async");
var http = require("http");
//var url = require("url");

// Overall test outcome
var ok=true;

var urlPrefix;

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

function doGet(path, next) {
  var len = 0;
  var data = "";
  http.get(urlPrefix+path, function(res) {
    res.on('data', function(d) { len += d.length; data = d; });
    res.on('close', function() {
      // We expect all responses to end with "END"
      var l = data.length;
      expect("get END", l < 3 || data.substring(l-3, l) === "END", data);
      next(len);
    });
  });
}

function doPost(path, data, next) {
  var respData = "";
  var q = url.parse(urlPrefix+path, false);
  q.method = "POST";
  q.headers = {'Content-Length': data.length};
  var req = http.request(q, function(resp) {
    resp.on('data', function(d) { respData += d; });
    resp.on('close', function() { next(data.length, respData); });
  });
  console.log("...");
  req.on('drain', function() {console.log("drain"); });
  req.end(data);
  req.on('error', function(err) { console.log("http request error", err); next(data.length, ""); } );
}

function testSock1() {
  async.sequence([
    // make a simple HTTP request
    function() {
      doGet("/ping", this.next);
    },
    function(len) {
      expect("ping", len === 8, len);
      this.next();
    },

    // make a simple HTTP request with explicit DNS lookup
    function() {
      wifi.getHostByName(test_host, this.next);
    },
    function(ip) {
      var tmp = urlPrefix;
      doGet("/ping", this.next);
      urlPrefix = tmp;
    },
    function(len) {
      expect("ping", len === 8, len);
      this.next();
    },

    // make an HTTP request with a long response
    function() {
      doGet("/data?size=3048", this.next);
    },
    function(len) {
      expect("data 3048", len == 3048, len);
      this.next();
    },

    // make an HTTP request with a long request
    function() {
      var data = "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
      data = data + data;
      doPost("/data", data, this.next);
    },
    function(len, resp) {
      expect("post", parseInt(resp,10) == len, resp);
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
  urlPrefix = "http://" + test_host + ":" + test_port;
  testSock1();
  console.log(process.memory());
}

