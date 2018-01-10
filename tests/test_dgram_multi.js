// Socket server and client test

var success = 0;
var result = 0;
var port1 = 41234;
var port2 = 51234;
let dgram = require('dgram');

let srv1 = dgram.createSocket('udp4');
srv1.bind(port1, function(bsrv) {
  bsrv.on('message', function(msg, info) {
    console.log("<"+JSON.stringify(msg));
    console.log("<"+JSON.stringify(info));
    bsrv.send(msg+'!', info.port, info.address);
  });
});
srv1.on('close', function() {
  console.log('server 1 disconnected');
});

let srv2 = dgram.createSocket('udp4');
srv2.bind(port2, function(bsrv) {
  bsrv.on('message', function(msg, info) {
    console.log("<"+JSON.stringify(msg));
    console.log("<"+JSON.stringify(info));
    bsrv.send(msg+'!', info.port, info.address);
  });
});
srv2.on('close', function() {
  console.log('server 2 disconnected');
});

let client = dgram.createSocket('udp4');
client.on('message', function(msg, info) {
  console.log(">"+JSON.stringify(msg));
  console.log(">"+JSON.stringify(info));

  if (msg=="42!" && info.address=="127.0.0.1" && info.port==port1) {
    success++;
    srv1.close();
  }
  if (msg=="24!" && info.address=="127.0.0.1" && info.port==port2) {
    success++;
    srv2.close();
  }

  result = success == 2 ? 1 : 0;
  if (result == 1) {
    clearTimeout(); // stop the fail fast
    client.close();
  }
});
client.on('close', function() {
  console.log('client disconnected');
});

// fail the test fast if broken
setTimeout(function() {
  client.close();
  srv1.close();
  srv2.close();
}, 100);

client.send('42', port1, 'localhost');
client.send('___24___', 3, 2, port2, 'localhost');
