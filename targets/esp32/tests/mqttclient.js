var ssid="RASPI3";
var password="password";

var wifi=require("Wifi");
wifi.connect(ssid, {password: password}, function() {
  console.log("Connected to access point");
  var mqtt = require("MQTT").create("192.168.5.1");
  mqtt.on("connected", function() {
    console.log("MQTT connected");
    mqtt.subscribe("test");
    mqtt.on('publish', function (pub) {
      console.log("topic: "+pub.topic);
      console.log("message: "+pub.message);
    });
  });
  console.log("Doing a connect");
  mqtt.connect();
});