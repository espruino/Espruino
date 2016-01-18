function test() {
  var req = require("http").get("http://h.voneicken.com:1234/ping", function(res) {
    res.on('data', function(data) { console.log("DATA <" + data + ">"); });
    res.on('close', function(e) { console.log("RES CLOSED, had error: ", e); });
    res.on('error', function(e) { console.log("RES ERROR: ", e); });
    console.log("RES:", res);
  });
  req.on("error", function(e) {
    console.log("REQ ERROR: ", e);
  });
  req.on("close", function(e) {
    console.log("REQ CLOSED, had error: ", e);
  });
  console.log("REQ:", req);
}
test();
