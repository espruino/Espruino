// Espruino esp8266 Wifi tester - Copyright 2015 by Thorsten von Eicken
//
// Test misc

var test_ssid = "tve-home";    // <====== set to your AP
var test_pwd  = "xxxxxxxx";    // <====== set to your AP

var async = require("Async");

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

function testCB(msg, next, timeout) {
  if (timeout === undefined) timeout = 1000;
  console.log("Wait for", msg, "in", timeout/1000.0, "sec");
  var n = 0;
  setTimeout(function() {
    if (n++ === 0) { console.log("OOPS: no callback for " + msg); ok = false; next(); }
  }, timeout);
  return function(x) { console.log("GOT CB for " + msg); if (n++ === 0) next(x); };
}

var wifi = require("Wifi");
var esp8266 = require("ESP8266");

function testMisc() {
  async.sequence([
    // connect so we can run tests
    function() {
      wifi.connect(test_ssid, {password: test_pwd}, this.next);
      console.log("Connecting to", test_ssid, "...");
    },

    // successful getHostByName
    function() {
      wifi.getHostByName("www.google.com", this.next);
    },
    function(ip) {
      expect("ip addr", ip !== null && ip.length > 6 && ip.split('.').length === 4, ip);
      this.next();
    },

    // failed getHostByName
    function() {
      wifi.getHostByName("www.google.doesnotexist", this.next);
    },
    function(ip) {
      expect("no ip", ip === null, ip);
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

wifi.setConfig({phy:"11n", powersave:"ps-poll"});
wifi.setConfig({phy:"11g"});
expectProps("11g", wifi.getStatus(), {phy:"11g", powersave:"ps-poll"});
wifi.setConfig({powersave:"none"});
expectProps("ps-off", wifi.getStatus(), {phy:"11g", powersave:"none"});
wifi.setConfig({phy:"11n", powersave:"ps-poll"});
expectProps("11n/ps-poll", wifi.getStatus(), {phy:"11n", powersave:"ps-poll"});

wifi.stopAP();
wifi.disconnect(testMisc);

console.log(process.memory());
