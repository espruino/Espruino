// Espruino esp8266 Socket tester - Copyright 2015 by Thorsten von Eicken
// HTTP client test from tests/test_http_*.js

test_host = "h.voneicken.com";
test_port = 4567;
require("ESP8266").setLog(2);

var async = require("Async");
var http = require("http");

var ok = true;
var host;

// barf if false
function expect(what, outcome, value) {
  if (!outcome) {
    console.log("In " + what + " unexpected value:\n", JSON.stringify(value), "\nOooops!");
    ok = false;
  } else {
    console.log("OK " + what);
  }
}

function testHttpSimple() {
  async.sequence([
    // make a simple HTTP request
    function() {
      console.log("** simple");
      var self = this;
      http.get(host+"/ping", function(res) {
        //console.log("Got response: " + JSON.stringify(res));
        res.on('data', function(data) {
          //console.log(">" + data +"<");
          expect("response", data=="pong\nEND", data);
          self.next();
        });
      });//.on('error', function(e) { console.log("Got error: " + e.message); });
    },

    // make an http request without data callback
    function() {
      console.log("** no cb");
      var self = this;
      http.get(host+"/ping", function(res) {
        //console.log("Got response: " + JSON.stringify(res));
        res.on('close', function(data) {
          //console.log("Closed");
          self.next();
        });
      });
    },

    // make an http request receiving an empty response
    function() {
      console.log("** empty");
      var self = this;
      var d = false;
      http.get(host+"/empty", function(res) {
        //console.log("Got response: " + JSON.stringify(res));
        res.on('data', function(data) {
          d = true; // got the CB
        });
        res.on('close', function() {
          expect("no data CB", !d, d);
          self.next();
        });
      });
    },

    // make an http request receiving a 404 response
    function() {
      console.log("** 404");
      var self = this;
      var d = false;
      var e1 = false;
      var e2 = false;
      var req = http.get(host+"/foobar", function(res) {
        //console.log("Got response: " + JSON.stringify(res));
        res.on('data', function(data) { d = true; }); // should get the 404 message
        res.on('error', function(err) { e1 = true; console.log("error", err); });
        res.on('close', function(hadError) {
          console.log("Closed", hadError);
          expect("data CB", d, d);
          expect("error CB1", !e1, e1);
          expect("error CB2", !e2, e2);
          expect("hadError", !hadError, hadError);
          expect("code 404", res.statusCode == 404, res);
          self.next();
        });
      });
      req.on('error', function(err) {
        console.log("error", err);
        e2 = true;
      });
    },

    ],
    {},
    function() {
      console.log("=== test ended:", (ok?"SUCCESS":"ERROR"),"===");
      console.log(process.memory());
    });
}

console.log("***** TEST START *****");
console.log(process.memory());

if (typeof test_host === 'undefined' || typeof test_port === 'undefined') {
  console.log("please set test_host, and test_port");
} else {
  host = "http://"+test_host+":"+test_port;
  testHttpSimple();
}
