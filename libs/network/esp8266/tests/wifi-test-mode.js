// Espruino esp8266 Wifi tester - Copyright 2015 by Thorsten von Eicken

var test_ssid = "tve-home"; // <<====== PUT YOUR SSID HERE
var test_pwd  = "XXXXXXXX"; // <<====== PUT YOUR NETWORK PASSWORD HERE

var async = require("Async");

// checks whether object x has all the properties and their values from object y
// this is a shallow comparison and fails for complex values of y
function hasProperties(x, y) {
  if (!(x instanceof Object)) { return false; }
  if (!(y instanceof Object)) { return false; }
  // properties equality check
  var p = Object.keys(x);
  return Object.keys(y).every(function(i) { return p.indexOf(i) !== -1 && x[i] == y[i]; });
}

// expect that a have all the properties of b, puts "what" into the output
function expectProperties(what, a, b) {
  if (hasProperties(a,b))
    console.log("OK for " + what);
  else
    console.log("OOPS: In " + what + " expected:\n", a, "\nto have:\n", b, "\n");
}

// expect an exception
function expectException(fun) {
  var exc = false;
  try { fun(); } catch(e) { exc = true; }
  if (!exc) console.log("OOPS: No exception for", fun);
}

// expect outcome==true and print value if not
function expect(what, outcome, value) {
  if (!outcome) console.log("In " + what + " unexpected value:\n", value, "\nOooops!");
  else console.log("OK " + what);
}

// test that a callback occurs
function testCB(msg, next, timeout) {
  if (timeout === undefined) timeout = 1000; // in ms
  console.log("Wait for", msg, "in", timeout/1000.0, "sec");
  var gotCB = false;
  // timeout for the callback so we don't wait forever if there's a failure
  setTimeout(function() {
    if (!gotCB) {
      gotCB = true; // prevent late CB from calling next
      console.log("OOPS: no callback for " + msg);
      next();
    }
  }, timeout);
  // this is the callback function we're registering
  return function() {
    var g = gotCB;
    gotCB = true; // we did get called back
    console.log("GOT CB for " + msg);
    if (!g) next(); // the !g prevents calling next if we timed out
  };
}

var wifi = require("Wifi");
var esp8266 = require("ESP8266");

wifi.on("connected", function(ev){console.log("CONNECTED!", ev);});

function testAPOnoff() {
  wifi.stopAP();
  wifi.disconnect();
  expectProperties("Wifi off", wifi.getStatus(),
                   {mode:"off", station:"off", ap:"disabled",
                    phy:"11n", powersave:"ps-poll", savedMode:"off"});
  expectProperties("Wifi off STA details", wifi.getDetails(), {status:"off"});
  expectProperties("Wifi off STA IP", wifi.getIP(), {ip:"0.0.0.0", netmask:"0.0.0.0"});
  expectProperties("Wifi off AP details", wifi.getAPDetails(), {status:"disabled", maxConn:4});
  expectProperties("Wifi off AP IP", wifi.getAPIP(), {ip:"0.0.0.0"});

  async.sequence([
    // start access point and check what we get
    function() {
      wifi.startAP("test1", null, testCB("startAP", this.next));
      expectProperties("Wifi AP", wifi.getStatus(), {mode:"ap", station:"off", ap:"enabled"});
      var apd = wifi.getAPDetails();
      expectProperties("Wifi AP", apd,
                       {status:"enabled", ssid:"test1", password:"", authMode:"open",
                        hidden:false, maxConn:4, savedSsid:null});
      expect("Wifi AP stations", apd.stations.length === 0, apd.stations);
      var ip = wifi.getAPIP();
      expectProperties("Wifi AP", ip, {ip:"192.168.4.1", netmask:"255.255.255.0", gw:"192.168.4.1"});
      expect("Wifi AP mac", ip.mac.length === 17, ip.mac);
    },

    // try a good connection and expect to get connected
    function() {
      wifi.connect(test_ssid, {password:test_pwd}, testCB("connect", this.next, 20000));
    },
    function() {
      expectProperties("Wifi STA+AP", wifi.getStatus(), {mode:"sta+ap", station:"connected", ap:"enabled"});
      var d = wifi.getDetails();
      expectProperties("Wifi STA details", d, {status:"connected", ssid:test_ssid, password:test_pwd, savedSsid:null});
                                               //authMode:"wpa2",
      expect("Wifi STA rssi", d.rssi < -20 && d.rssi > -110, d.rssi);
      var ip = wifi.getIP();
      expect("Wifi STA ip", ip.ip.length > 6, ip.ip);
      expect("Wifi STA mask", ip.netmask.length > 6, ip.netmask);
      expect("Wifi STA gw", ip.gw.length > 6, ip.gw);
      this.next();
    },

    // repeat the identical connect, should produce immediate callback
    function() {
      wifi.connect(test_ssid, {password:test_pwd}, testCB("connect #2", this.next));
    },
    function() {
      expectProperties("Wifi STA+AP #2", wifi.getStatus(), {mode:"sta+ap", station:"connected", ap:"enabled"});
      this.next();
    },

    // stop the AP
    function() {
      wifi.stopAP(testCB("Wifi STA", this.next));
      expectProperties("Wifi STA", wifi.getStatus(), {mode:"sta", station:"connected", ap:"disabled"});
    },

    // scan the network
    function() {
      var self = this;
      wifi.scan(function(aps) {
        var found = false;
        aps.every(function(ap) {
          if (ap.ssid != test_ssid) return;
          found = true;
          expect("Wifi scan ch", ap.channel > 0, ap.channel);
        });
        expect("Wifi scan found", found, aps);
        self.next();
      });
    }],
    {},
    function() {
      console.log("OK AP OnOff");
    });
}

console.log("***** TEST START *****");
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

setTimeout(testAPOnoff, 1000);
console.log("=== real test ===");










console.log(process.memory());
