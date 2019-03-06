// HTTP Headers longer than MSS server and client test

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
    res.writeHead(200);
    res.end('42'+body+req.headers['X-Check']);
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
  headers: {
    'X-LongHeader': new Array(35).fill('long header item').join(','),
    'X-Check': '24'
  }
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

req.end('-0123456789abcdef-'); // longer than 15 chars
