// Socket server and client test

var result = 0;
var port = 41234;
let dgram = require('dgram');

let srv1 = dgram.createSocket({ type: 'udp4', reuseAddr: true });
srv1.bind(port);
let srv2 = dgram.createSocket({ type: 'udp4', reuseAddr: true });
srv2.bind(port);

try {
    let srv3 = dgram.createSocket({ type: 'udp4', reuseAddr: false });
    srv3.bind(port);
} catch(e) {
    // the reuseAddr needs to be set for all servers
    result = 1;
}

srv1.close();
srv2.close();
srv3.close();
