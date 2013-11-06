http.createServer(function (req, res) {
  res.writeHead(200);
  res.end();
}).listen(8080);

setTimeout("print('done');", 10000000);
