// Socket server and client test

var result = 0;
var net = require("net");

var server = net.createServer(function(c) { //'connection' listener
  c.write("42");
  c.end();
});
server.listen(4444);

var client = net.connect({port: 4444}, function() { //'connect' listener
  console.log('client connected');
  client.on('data', function(data) {
    console.log(">"+JSON.stringify(data));
    result = data=="42";
    server.close();
  });
  client.on('end', function() {
    console.log('client disconnected');
  });
});

