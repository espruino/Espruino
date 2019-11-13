// HTTP Transfer-Encoding: chunked server and client test

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
    console.log("<req", req.headers, body);
    res.writeHead(200, {'Content-Type': 'text/plain', 'Transfer-Encoding': 'chunked' });
    res.write('42');
    setTimeout(() => {
      res.write(body);
      res.end();
    }, 10);
  });
  req.on('close', function() {
    console.log("<close");
  });
  req.on('error', function(e) {
    console.log("<error" + e.message);
  });
});
server.listen(8080);

// payload longer than 66 chars (parseInt limit)
//                and MSS (536) for partialChunk testing
var payload = new Array(40).fill('-0123456789abcdef-').join('');
var options = {
  host: 'localhost',
  port: 8080,
  path: '/post.html',
  method: 'POST',
  protocol: 'http:',
  headers: {
    'Transfer-Encoding': 'Chunked'
  }
};
var req = http.request(options, function(res) {
  console.log(">RES", res.headers);
  var body = '';
  res.on('data', function(data) {
    console.log(">" + data);
    body += data;
  });
  res.on('end', function() {
    server.close();
    result = body==("42"+payload+"24");
    console.log(">END", result);
  });
  res.on('close', function() {
    console.log(">CLOSE");
  });
})

req.on('error', function(e) {
  console.log(">ERROR" + e.message);
});

req.write(payload);
setTimeout(() => {
  req.write('24');
  req.end();
}, 10)

