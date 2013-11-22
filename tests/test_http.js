// HTTP server and client test

var result = 0;
var timeout = setTimeout("print('done');", 10000000);
var http = require("http");

http.createServer(function (req, res) {
  console.log("Connected " + JSON.stringify(req));
  res.writeHead(200, {'Content-Type': 'text/plain'});
  res.write('42');
  res.end();
}).listen(8080);

http.get("http://localhost:8080/foo.html", function(res) {
  console.log("Got response: " + JSON.stringify(res));
  res.on('data', function(data) {
	  console.log(">" + data);
          result = data=="42";
          clearTimeout(timeout);
	});
});//.on('error', function(e) {
//  console.log("Got error: " + e.message);
//});*/

