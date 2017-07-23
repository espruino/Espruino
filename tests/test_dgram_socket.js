// Socket server and client test

var result = 0;
var port = 41234;
let dgram = require('dgram');

let srv = dgram.createSocket('udp4');
srv.bind(port, function(bsrv) {
  bsrv.on('message', function(msg, info) {
    console.log("<"+JSON.stringify(msg));
    console.log("<"+JSON.stringify(info));
    bsrv.send(msg+'!', info.port, info.address);
  });
});
srv.on('close', function() {
  console.log('server disconnected');
});

let client = dgram.createSocket('udp4');
client.on('message', function(msg, info) {
  console.log(">"+JSON.stringify(msg));
  console.log(">"+JSON.stringify(info));

  result = msg=="42!" && info.address=="127.0.0.1" && info.port==port;

  clearTimeout(); // stop the fail fast

  srv.close();
  client.close();
});
client.on('close', function() {
  console.log('client disconnected');
});

// fail the test fast if broken
setTimeout(function() {
  client.close();
  srv.close();
}, 100);

client.send('42', port, 'localhost');
