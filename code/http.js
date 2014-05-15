/*
require('http').createServer(function (req, res) {
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

require('http').get("http://www.pur3.co.uk", function(res) {
  console.log("Got response: " + JSON.stringify(res));
  res.on('data', function(data) {
	  console.log("->" + data);
	});
  res.on('close', function(data) {
	  console.log("[[[CLOSED]]]");
	});
});/*.on('error', function(e) {
  console.log("Got error: " + e.message);
});*/

setTimeout("print('done');", 10000000);



function onPageRequest(req, res) {
  req.on('data', function(d) { console.log(">>> "+JSON.stringify(d)); });
  res.writeHead(200, {'Content-Type': 'text/html'});
  res.write('<html><body><form method="post">');
  res.write('<textarea name="foo">Hello There</textarea><input type="submit">');
  res.end('</form></body></html>');
}
require('http').createServer(onPageRequest).listen(8080);


function onPageRequest(req, res) {
  res.writeHead(200, {'Content-Type': 'video/webm'});
  new File("rick.webm").pipe(res);
}
require('http').createServer(onPageRequest).listen(8080);


/* Outputting more data than you have RAM - push more data
on the drain event. Call 'end' when you're done */
function onPage(req, res) {
  res.writeHead(200, {'Content-Type': 'text/plain'});
  var n = 1;
  res.on('drain',function() {
    for (var i=0;i<10;i++)
      res.write((n++)+",");
    if (n>10000) res.end();
  });
  res.write("Start:");
}


/// send an array
var history = new Uint8Array(1000);

function onPage(req, res) {
  res.writeHead(200, {'Content-Type': 'text/plain'});
  var n = 0;
  res.on('drain',function() {
    for (var i=0;i<10;i++)
      res.write(history[n++]+",");
    if (n>=history.length) res.end(" ];");
  });
  res.write("var data = [");
}
