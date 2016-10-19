// Test ESP32 WiFi event callbacks
var ssid="RASPI3";
var password="password";

var wifi=require("Wifi");
wifi.on("associated", function(details) {
	console.log("Event-> associated: " + JSON.stringify(details));
});
wifi.on("connected", function(details) {
	console.log("Event-> connected: " + JSON.stringify(details));
});
wifi.on("disconnected", function(details) {
	console.log("Event-> disconnected: " + JSON.stringify(details));
});
wifi.on("auth_change", function(details) {
	console.log("Event-> auth_change: " + JSON.stringify(details));
});
wifi.on("dhcp_timeout", function(details) {
	console.log("Event-> dhcp_timeout: " + JSON.stringify(details));
});
wifi.on("probe_recv", function(details) {
	console.log("Event-> probe_recv: " + JSON.stringify(details));
});
wifi.on("sta_joined", function(details) {
	console.log("Event-> sta_joined: " + JSON.stringify(details));
});
wifi.on("sta_left", function(details) {
	console.log("Event-> sta_left: " + JSON.stringify(details));
});
wifi.connect(ssid, {password: password}, function() {
	console.log("Connect completed");
});