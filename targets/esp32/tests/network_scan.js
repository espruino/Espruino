var wifi=require("Wifi");
wifi.scan(function(data) {
  for (var i=0; i<data.length; i++) {
    console.log(JSON.stringify(data[i]));
  }
});