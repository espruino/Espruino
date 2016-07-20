// Espruino esp8266 Wifi tester - Copyright 2015 by Thorsten von Eicken
//
// Test #on events

var test_ssid = "tve-home";    // <========= configure for your AP
var test_pwd  = "XXXXXXXX";    // <========= configure for your AP

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

var nextFun = null; // "next" function of async.sequence

function onWifi(ev) {
  wifi.on(ev, function(d) {
    console.log("EV:", ev);
    if (nextFun !== null) nextFun(ev, d);
  });
}

onWifi("associated");
onWifi("connected");
onWifi("disconnected");
onWifi("auth_change");
onWifi("dhcp_timeout");
onWifi("sta_joined");
onWifi("sta_left");
onWifi("probe_recv");

function testEvents() {
  async.sequence([
    function() { nextFun = this.next.bind(this); nextFun(); }, // get set-up

    // connect with no password, expect disconnect to repeat
    function() {
      wifi.connect(test_ssid, {});
      console.log("Connecting to", test_ssid, "...");
    },
    function(ev, d) {
      expect("badpwd1", ev === "disconnected", ev);
      expectProps("badpwd", d, {reason:"4way_handshake_timeout", ssid:test_ssid});
    },
    function(ev, d) {
      expect("badpwd2", ev === "disconnected", ev);
      expectProps("badpwd", d, {reason:"4way_handshake_timeout", ssid:test_ssid});
      this.next();
    },

    // connect with password, expect connect
    function() {
      console.log("connect #2 starting");
      wifi.connect(test_ssid, {password:test_pwd});
    },
    function(ev, d) {
      expect("conn2.1", ev === "associated", ev);
      expectProps("comm2.1", d, {ssid:test_ssid});
    },
    function(ev, d) {
      expect("conn2.3", ev === "connected", ev);
      expectProps("conn2.3", d, {mask:"255.255.255.0"});
      this.next();
    },

    // disconnect
    function() {
      wifi.disconnect();
    },
    function(ev, d) {
      expect("discon", ev === "disconnected", ev);
      expectProps("discon", d, {reason:"assoc_leave", ssid:test_ssid});
      this.next();
    },

    // start AP and expect to see a client connect and then disconnect
    function() {
      wifi.startAP("esp-test");
      console.log("PLEASE join the network esp-test now!");
    },
    function(ev, d) {
      expect("probe", ev === "probe_recv", ev);
      expect("probe mac", d.mac.length === 17, d);
      wifi.removeAllListeners("probe_recv");
    },
    function(ev, d) {
      expect("join", ev === "sta_joined", ev);
      expect("join mac", d.mac.length === 17, d);
      console.log("PLEASE leave the network esp-test now!");
    },
    function(ev, d) {
      expect("leave", ev === "sta_left", ev);
      expect("leave mac", d.mac.length === 17, d);
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

wifi.stopAP();
wifi.disconnect();
setTimeout(function() {
  testEvents();
}, 1000);

console.log(process.memory());
