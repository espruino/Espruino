// Espruino esp8266 Socket tester - Copyright 2015 by Thorsten von Eicken

// Test error handling

test_host = "h.voneicken.com";  // <======= configure for your local server running the test_http.rb sinatra server
test_port = 4567;

var test_ip = 0; // will be resolved

// Test misc
var wifi = require("Wifi");
var esp8266 = require("ESP8266");
var async = require("Async");
var http = require("http");
var net = require("net");

// Overall test outcome
var ok=true;

function hasProps(x, y) {
  if (!(x instanceof Object)) { return false; }
  if (!(y instanceof Object)) { return false; }
  // properties equality check
  var p = Object.keys(x);
  return Object.keys(y).every(function(i) { return p.indexOf(i) !== -1 && x[i] == y[i]; });
}

function expectProps(what, a, b) {
  if (hasProps(a,b)) {
    console.log("OK for " + what);
  } else {
    console.log("OOPS: In " + what + " expected:\n", a, "\nto have:\n", b, "\n");
    ok = false;
  }
}

// barf if false
function expect(what, outcome, value) {
  if (!outcome) {
    console.log("In " + what + " unexpected value:\n", value, "\nOooops!");
    ok = false;
  } else {
    console.log("OK " + what);
  }
}

function testSockError() {
  async.sequence([

    // connection reset
    function() {
      console.log("** reset");
      var self = this;
      var err = false;
      var conn = 0;
      var sock = net.connect({host:test_host, port:33}, function(){conn++;});
      sock.on('close', function(hadError) {
        expect("got error", err, err);
        expect("hadError", hadError, hadError);
        self.next();
      });
      sock.on('error', function(ev) {
        expectProps("conn reset", ev, {code: -9, message: "connection reset"});
        expect("no conn", conn === 0, conn);
        err = true;
      });
      sock.on('connect', function(ev) {
        expect("conn cb undefined", ev === undefined, ev);
        conn++;
      });
    },

    // host not found
    function() {
      console.log("** not found");
      var self = this;
      var err = false;
      var sock = net.connect({host:"i-do-not-exist.tada", port:33}, function(){conn++;});
      sock.on('close', function(hadError) {
        expect("got error", err, err);
        self.next();
      });
      sock.on('error', function(ev) {
        expectProps("not found", ev, {code: -6, message: "not found"});
        err = true;
      });
    },

    // connection closed
    function() {
      console.log("** closed");
      var self = this;
      var err = false;
      var conn = 0;
      var drain = 0;
      var sock = net.connect({host:test_host, port:22});
      sock.on('drain', function() { drain++; });
      sock.on('close', function(hadError) {
        expect("got error", err, err);
        expect("hadError", hadError, hadError);
        expect("1 connCB", conn === 1, conn);
        expect("2 drainCB", drain === 2, drain);
        self.next();
      });
      sock.on('error', function(e) {
        expectProps("conn reset", e, {code: -9, message: "connection reset"});
        err = true;
      });
      sock.on('connect', function() { conn++; sock.write("GARBAGE\r\n\r\n"); });
    },

    // unsent data
    function() {
      console.log("** unsent");
      var self = this;
      var err = false;
      var sock = net.connect({host:test_host, port:4567});
      sock.on('drain', function() { sock.write("GARBAGE\r\n\r\n"); });
      sock.on('close', function() {
        expect("got error", err, err);
        self.next();
      });
      sock.on('error', function(e) {
        expectProps("unsent", e, {code: -8, message: "unsent data"});
        err = true;
      });
    },

    // connection refused
    function() {
      this.next(); return;
      var self = this;
      var req = http.request(url.parse("http://"+test_host+":33/"), function(res) {
        res.on('close', function() { console.log("closed"); });
      });
      req.on('error', function(err) {
        console.log("Got error:", err);
        self.next();
      });
      req.end();
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
  testSockError();
  console.log(process.memory());
}

