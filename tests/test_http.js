// HTTP server and client test

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

http.get("http://localhost:8080/foo.html", function(res) {
  console.log("Got response: " + JSON.stringify(res));
  res.on('data', function(data) {
	  console.log(">" + data);
          result = data=="42";
          server.close();
	});
});//.on('error', function(e) {
//  console.log("Got error: " + e.message);
//});*/

