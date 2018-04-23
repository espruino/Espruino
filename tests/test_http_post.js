// HTTP POST 'data' server and client test

var result = 0;
var http = require("http");

var server = http.createServer(function (req, res) {
  console.log("Connected");
  var body = '';
  req.on('data', function(data) {
    console.log("<" + data);
    body += data;
  });
  req.on('end', function() {
    console.log("<end");
    console.log("<req ", req.headers, body);
    res.writeHead(200, {'Content-Type': 'text/plain' });
    res.end('42'+body);
  });
  req.on('close', function() {
    console.log("<close");
  });
  req.on('error', function(e) {
    console.log("<error: " + e.message);
  });
});
server.listen(8080);

var options = {
  host: 'localhost',
  port: 8080,
  path: '/post.html',
  method: 'POST',
  protocol: 'http:',
};
var req = http.request(options, function(res) {
  console.log(">RES ", res.headers);
  var body = '';
  res.on('data', function(data) {
    console.log(">" + data);
    body += data;
  });
  res.on('end', function() {
    console.log(">END");
    server.close();
    result = body=="42-0123456789abcdef-24";
  });
  res.on('close', function() {
    console.log(">CLOSE");
  });
})

req.on('error', function(e) {
  console.log(">ERROR: " + e.message);
});

req.end('-0123456789abcdef-24'); // longer than 15 chars
