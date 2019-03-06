// Socket server and client test

var result = 0;
var port = 41234;
let dgram = require('dgram');

let payload = new Array(100).fill('-42-22-24-').join('')

let srv = dgram.createSocket({ type: 'udp4', recvBufferSize: 1024 });
srv = srv.bind(port, function() {
  srv.on('message', function(msg, info) {
    console.log("<", msg.length, JSON.stringify(msg.substring(msg.length-4)));
    console.log("<"+JSON.stringify(info));
    srv.send(msg+'!', info.port, info.address);
  });
});
srv.on('close', function() {
  console.log('server disconnected');
});

let client = dgram.createSocket('udp4');
client.on('message', function(msg, info) {
  console.log('>', msg.length, JSON.stringify(msg.substring(msg.length-5)));
  console.log(">"+JSON.stringify(info));
  result =  msg.substring(msg.length-5)==="-24-!" &&
           (msg.length === payload.length+1) &&
           info.address=="127.0.0.1" && info.port==port;

  clearTimeout(failTimeout); // stop the fail fast

  srv.close();
  client.close();
});
client.on('close', function() {
  console.log('client disconnected');
});

// fail the test fast if broken
failTimeout = setTimeout(function() {
  client.close();
  srv.close();
}, 100);

console.log('P>', payload.length);
client.send(payload, port, 'localhost');
