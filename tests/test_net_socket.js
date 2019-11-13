// Socket server and client test

var result = 0;
var connectCount = 0;
var net = require("net");

var server = net.createServer(function(c) { //'connection' listener
  c.write("42");
  c.end();
});
server.listen(4444);

var client = net.connect({port: 4444}, function() { //'connect' listener
  var body='';
  console.log('client connected');
  connectCount++;

  client.on('data', function(data) {
    console.log(">"+JSON.stringify(data));
    body += data;
    server.close();
  });
  client.on('end', function() {
    console.log('client disconnected');
    result = body=="42" && connectCount==1;
  });
});

