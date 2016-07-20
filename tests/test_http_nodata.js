// HTTP server and client test WITH NO data handler

var result = 0;
var http = require("http");

var server = http.createServer(function (req, res) {
  console.log("Connected");
  console.log("Request " + JSON.stringify(req));
  console.log("Response " + JSON.stringify(res));
  res.writeHead(200, {'Content-Type': 'text/plain'});
  res.write('42');
  res.end();
});
server.listen(8080);
setTimeout("server.close();", 100);

http.get("http://localhost:8080/foo.html", function(res) {
  console.log("Got response: " + JSON.stringify(res));
  res.on('close', function(data) {
          console.log("Closed");
          result = 1;
          clearTimeout();
          server.close();
	});
});


