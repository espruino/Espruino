// Espruino esp8266 Wifi tester - Copyright 2015 by Thorsten von Eicken

var test_ssid = "tve-home"; // <===== configure for your AP!
var test_pwd  = "xxxxxxxx";

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

// expect an exception
function expectException(fun) {
  var exc = false;
  try { fun(); } catch(e) { exc = true; }
  if (!exc) { console.log("OOPS: No exception for", fun); ok = false; }
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

// check that the set of access points found in the scan includes the test_ssid network
// and that its channel number is provided. Calls this.next() when done, so use something
// like wifi.scan(checkScan.bind(this)) to ensure the correct binding of this.
function checkScan(aps) {
  var found = false;
  aps.every(function(ap) {
    if (ap.ssid != test_ssid) return;
    found = true;
    expect("scan ch", ap.channel > 0, ap.channel);
  });
  expect("scan found", found, aps);
  this.next();
}

var wifi = require("Wifi");
var esp8266 = require("ESP8266");

function testAPOnoff() {
  wifi.stopAP();
  wifi.disconnect();
  expectProps("off", wifi.getStatus(),
                   {mode:"off", station:"off", ap:"disabled",
                    phy:"11n", savedMode:"off"});
  expectProps("off STA details", wifi.getDetails(), {status:"off"});
  expectProps("off STA IP", wifi.getIP(), {ip:"0.0.0.0", netmask:"0.0.0.0"});
  expectProps("off AP details", wifi.getAPDetails(), {status:"disabled", maxConn:4});
  expectProps("off AP IP", wifi.getAPIP(), {ip:"0.0.0.0"});


  async.sequence([
    // start access point and check what we get
    function() {
      wifi.startAP("test1", null, testCB("startAP", this.next));
      expectProps("AP", wifi.getStatus(), {mode:"ap", station:"off", ap:"enabled"});
      var apd = wifi.getAPDetails();
      expectProps("AP", apd,
                       {status:"enabled", ssid:"test1", password:"", authMode:"open",
                        hidden:false, maxConn:4, savedSsid:null});
      expect("AP stations", apd.stations.length === 0, apd.stations);
      var ip = wifi.getAPIP();
      expectProps("AP", ip, {ip:"192.168.4.1", netmask:"255.255.255.0", gw:"192.168.4.1"});
      expect("AP mac", ip.mac.length === 17, ip.mac);
    },

    // try a good connection and expect to get connected
    function() {
      wifi.connect(test_ssid, {password:test_pwd}, testCB("connect", this.next, 20000));
    },
    function() {
      expectProps("STA+AP", wifi.getStatus(), {mode:"sta+ap", station:"connected", ap:"enabled"});
      var d = wifi.getDetails();
      expectProps("STA details", d, {status:"connected", ssid:test_ssid, password:test_pwd, savedSsid:null});
                                               //authMode:"wpa2",
      expect("STA rssi", d.rssi < -20 && d.rssi > -110, d.rssi);
      var ip = wifi.getIP();
      expect("STA ip", ip.ip.length > 6, ip.ip);
      expect("STA mask", ip.netmask.length > 6, ip.netmask);
      expect("STA gw", ip.gw.length > 6, ip.gw);
      this.next();
    },

    // repeat the identical connect, should produce immediate callback
    function() {
      wifi.connect(test_ssid, {password:test_pwd}, testCB("connect #2", this.next));
    },
    function() {
      expectProps("STA+AP #2", wifi.getStatus(), {mode:"sta+ap", station:"connected", ap:"enabled"});
      this.next();
    },

    // stop the AP
    function() {
      wifi.stopAP(testCB("STA", this.next));
      expectProps("STA", wifi.getStatus(), {mode:"sta", station:"connected", ap:"disabled"});
    },

    // scan the network
    function() {
      wifi.scan(checkScan.bind(this));
    },

    // stop the STA 2x and expect a callback 2x
    function() {
      wifi.disconnect(testCB("disconnect", this.next));
    },
    function() {
      wifi.disconnect(testCB("disconnect", this.next));
    },

    // connect with no password and expect error
    function() {
      wifi.connect(test_ssid, {}, testCB("badpwd", this.next, 20000));
    },
    function(err) {
      expect("badpwd", err=="bad password", err);
      this.next();
    },

    // connect with bad password and expect error
    function() {
      wifi.connect(test_ssid, {password: "foo"}, testCB("badpwd", this.next, 20000));
    },
    function(err) {
      expect("badpwd", err=="bad password", err);
      this.next();
    },

    // perform a scan and expect it to work, then expect that STA mode is on
    function() {
      wifi.scan(checkScan.bind(this));
    },
    function() {
      expectProps("STA", wifi.getStatus(), {mode:"sta", station:"bad_password", ap:"disabled"});
      this.next();
    },

    // call disconnect and expect callback even though we're not connected
    function() {
      wifi.disconnect(testCB("disconnect", this.next));
    },
    function() {
      expectProps("STA", wifi.getStatus(), {mode:"off", station:"off", ap:"disabled"});
      this.next();
    },

    ],
    {},
    function() {
      console.log("=== test ended:", (ok?"SUCCESS":"ERROR"));
    });
}

console.log("***** TEST START");
console.log(process.memory());

// test invalid parameter exceptions
expectException(function(){wifi.startAP();});
expectException(function(){wifi.startAP(1);});
expectException(function(){wifi.startAP("test1", 1);});
expectException(function(){wifi.startAP("test1", null, 1);});
expectException(function(){wifi.startAP("test1", {"password":"xxx", "authMode":"open"});});
expectException(function(){wifi.startAP("test1", {"password":"", "authMode":"wpa2"});});
expectException(function(){wifi.startAP("test1", {"authMode":"wpa2"});});
expectException(function(){wifi.stopAP(1);});
expectException(function(){wifi.disconnect(1);});
expectException(function(){wifi.connect();});
expectException(function(){wifi.connect(1);});
expectException(function(){wifi.connect("test1", 1);});
expectException(function(){wifi.connect("test1", null, 1);});
expectException(function(){wifi.scan();});
expectException(function(){wifi.scan(1);});
expectException(function(){wifi.setConfig();});
expectException(function(){wifi.setConfig({phy:"11a"});});
expectException(function(){wifi.setConfig({powersave:"foo"});});

setTimeout(function() {
  console.log("=== real test");
    wifi.setConfig({powersave:"ps-poll", phy:"11n"});
  wifi.stopAP();
  wifi.disconnect(testAPOnoff);
}, 1000);











console.log(process.memory());
