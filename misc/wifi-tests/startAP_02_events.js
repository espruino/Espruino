// AP test with join/leave event logging
var wifi = require("Wifi");

wifi.startAP("espruino-ap", { password: "test1234", authMode: "wpa2" }, function (err) {
  console.log("startAP err:", err);
  wifi.getAPIP(function (e, ip) {
    console.log("AP IP err:", e, "ip:", ip);
  });
});

wifi.on("associated", function (d) { console.log("associated:", d); });
wifi.on("connected", function (d) { console.log("connected:", d); });
wifi.on("disconnected", function (d) { console.log("disconnected:", d); });

// AP station events (softAP)
wifi.on("sta_joined", function (d) { console.log("sta_joined:", d); });
wifi.on("sta_left", function (d) { console.log("sta_left:", d); });

// probe requests (may be frequent/noisy)
wifi.on("probe_recv", function (d) { console.log("probe_recv:", d); });
