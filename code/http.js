/*http.createServer(function (req, res) {
  console.log("Connected");
  res.writeHead(200, {'Content-Type': 'text/plain'});
  res.write('Hello World - ');
  res.end(JSON.stringify(req)+'\n');
}).listen(8080);

*/

console.log("Parse: " + JSON.stringify(url.parse("http://localhost/foo.html")));
console.log("Parse: " + JSON.stringify(url.parse("http://www.pur3.co.uk")));
console.log("Parse: " + JSON.stringify(url.parse("http://localhost:82")));
console.log("Parse: " + JSON.stringify(url.parse("http://localhost:82/lala.html?jkhdgs")));

var options = {
  host: 'www.google.com',
  port: 80,
  path: '/index.html',
  method: 'GET'
};

var options = {
  host: 'localhost',
  port: 80,
  path: '/',
  method: 'GET'
};

http.get("http://www.pur3.co.uk", function(res) {
  console.log("Got response: " + JSON.stringify(res));
  res.on('data', function(data) {
	  console.log(">" + data);
	});
});/*.on('error', function(e) {
  console.log("Got error: " + e.message);
});*/

setTimeout("print('done');", 10000000);