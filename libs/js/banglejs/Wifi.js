var WIFI_BOOT = V11;

var MODE = { CLIENT : 1, AP : 2 };
var ENCR_FLAGS = ["open","wep","wpa_psk","wpa2_psk","wpa_wpa2_psk"];

var wifiMode = 0; // Uses MODE enum
var connected = 0; // Uses MODE enum
var at;
var socks = [];
var sockUDP = [];
var sockData = ["","","","",""];
var MAXSOCKETS = 5;

function udpToIPAndPort(data) {
  return {
    ip : data.charCodeAt(0)+"."+data.charCodeAt(1)+"."+data.charCodeAt(2)+"."+data.charCodeAt(3),
    port : data.charCodeAt(5)<<8 | data.charCodeAt(4),
    len : data.charCodeAt(7)<<8 | data.charCodeAt(6) // length of data
  };
}

/*
`socks` can have the following states:

undefined         : unused
true              : connected and ready
"DataClose"       : closed on esp8266, but with data still in sockData
"Wait"            : waiting for connection (client), or for data to be sent
"WaitClose"       : We asked to close it, but it hasn't been opened yet
"Accept"          : opened by server, waiting for 'accept' to be called
"UDP"             : reserved for UDP, but not yet opened
*/

// -----------------------------------------------------------------------------------
var netCallbacks = {
  create : function(host, port, type) {
    //console.log("CREATE ",arguments,connected);
    if (!at || !connected) return -1; // disconnected
    /* Create a socket and return its index, host is a string, port is an integer.
    If host isn't defined, create a server socket */
    if (host===undefined && type!=2) {
      var sckt = MAXSOCKETS;
      socks[sckt] = "Wait";
      sockData[sckt] = "";
      at.cmd("AT+CIPSERVER=1,"+port+"\r\n", 10000, function(d) {
        if (d=="OK") {
          socks[sckt] = true;
        } else {
          socks[sckt] = undefined;
          setTimeout(function() {
            throw new Error("CIPSERVER failed ("+(d?d:"Timeout")+")");
          }, 0);
        }
      });
      return MAXSOCKETS;
    } else {
      var sckt = 0;
      while (socks[sckt]!==undefined) sckt++; // find free socket
      if (sckt>=MAXSOCKETS) return -7; // SOCKET_ERR_MAX_SOCK
      sockData[sckt] = "";
      socks[sckt] = "Wait";
      var cmd;
      if (type==2) {
        // If there's a port specified, make a server now - otherwise reserve the socket and do it later
        if (port) cmd = 'AT+CIPSTART='+sckt+',"UDP","255.255.255.255",'+port+','+port+',2\r\n';
        else socks[sckt] = "UDP";
        sockUDP[sckt] = true;
      } else {
        cmd = 'AT+CIPSTART='+sckt+',"TCP",'+JSON.stringify(host)+','+port+'\r\n';
        delete sockUDP[sckt];
      }
      if (cmd) at.cmd(cmd,10000,function cb(d) {
        //console.log("CIPSTART "+JSON.stringify(d));
        if (d=="ALREADY CONNECTED") return cb; // we're expecting an ERROR too
        // x,CONNECT should have been received between times. If it hasn't appeared, it's an error.
        if (d!="OK" || socks[sckt]!==true) socks[sckt] = -6; // SOCKET_ERR_NOT_FOUND
      });
    }
    return sckt;
  },
  /* Close the socket. returns nothing */
  close : function(sckt) {
    //console.log("CLOSE ",arguments);
    if (socks[sckt]=="Wait")
      socks[sckt]="WaitClose";
    else if (socks[sckt]!==undefined) {
      // socket may already have been closed (eg. received 0,CLOSE)
      if (socks[sckt]<0 || socks[sckt]=="UDP")
        socks[sckt] = undefined;
      else
      // we need to a different command if we're closing a server
        at.cmd(((sckt==MAXSOCKETS) ? 'AT+CIPSERVER=0' : ('AT+CIPCLOSE='+sckt))+'\r\n',1000, function() {
          socks[sckt] = undefined;
        });
    }
  },
  /* Accept the connection on the server socket. Returns socket number or -1 if no connection */
  accept : function() {
    // console.log("Accept",sckt);
    for (var i=0;i<MAXSOCKETS;i++)
      if (socks[i]=="Accept") {
        //console.log("Socket accept "+i,JSON.stringify(sockData),JSON.stringify(socks));
        socks[i] = true;
        return i;
      }
    return -1;
  },
  /* Receive data. Returns a string (even if empty).
  If non-string returned, socket is then closed */
  recv : function(sckt, maxLen) {
    if (sockData[sckt]) {
      var r;
      if (sockData[sckt].length > maxLen) {
        r = sockData[sckt].substr(0,maxLen);
        sockData[sckt] = sockData[sckt].substr(maxLen);
      } else {
        r = sockData[sckt];
        sockData[sckt] = "";
        if (socks[sckt]=="DataClose")
          socks[sckt] = undefined;
      }
      return r;
    }
    if (socks[sckt]<0) return socks[sckt]; // report an error
    if (!socks[sckt]) return -1; // close it
    return "";
  },
  /* Send data. Returns the number of bytes sent - 0 is ok.
  Less than 0  */
  send : function(sckt, data, type) {
    if (!at) return -1; // disconnected
    if (at.isBusy() || socks[sckt]=="Wait") return 0;
    if (socks[sckt]<0) return socks[sckt]; // report an error
    if (!socks[sckt]) return -1; // close it
    //console.log("SEND ",arguments);
    var d;
    if (socks[sckt]=="UDP") {
      d = udpToIPAndPort(data);
      socks[sckt]="Wait";
      at.cmd('AT+CIPSTART='+sckt+',"UDP","'+d.ip+'",'+d.port+','+d.port+',2\r\n',10000,function(d) {
        if (d!="OK") socks[sckt] = -6; // SOCKET_ERR_NOT_FOUND
      });
      return 0;
    }
    //console.log("Send",sckt,data);
    var returnVal = data.length;
    var extra = "";
    if (type==2) { // UDP
      d = udpToIPAndPort(data);
      extra = ',"'+d.ip+'",'+d.port;
      data = data.substr(8,d.len);
      returnVal = 8+d.len;
    }

    var cmd = 'AT+CIPSEND='+sckt+','+data.length+extra+'\r\n';
    at.cmd(cmd, 2000, function cb(d) {
      //console.log("SEND "+JSON.stringify(d));
      if (d=="OK") {
        at.register('> ', function(l) {
          at.unregister('> ');
          at.write(data);
          return l.substr(2);
        });
      } else if (d=="Recv "+data.length+" bytes" || d=="busy s...") {
        // all good, we expect this
        // Not sure why we get "busy s..." in this case (2 sends one after the other) but it all seems ok.
      } else if (d=="SEND OK") {
        // we're ready for more data now
        if (socks[sckt]=="WaitClose") netCallbacks.close(sckt);
        socks[sckt]=true;
        return;
      } else {
        socks[sckt]=undefined; // uh-oh. Error. If undefined it was probably a timeout
        at.unregister('> ');
        return;
      }
      return cb;
    });
    // if we obey the above, we shouldn't get the 'busy p...' prompt
    socks[sckt]="Wait"; // wait for data to be sent
    return returnVal;
  }
};


//Handle +IPD input data from ESP8266
function ipdHandler(line) {
  var colon = line.indexOf(":");
  if (colon<0) return line; // not enough data here at the moment
  var parms = line.substring(5,colon).split(",");
  parms[1] = 0|parms[1];
  var len = line.length-(colon+1);
  var sckt = parms[0];
  if (sockUDP[sckt]) {
    var ip = (parms[2]||"0.0.0.0").split(".").map(function(x){return 0|x;});
    var p = 0|parms[3]; // port
    sockData[sckt] += String.fromCharCode(ip[0],ip[1],ip[2],ip[3],p&255,p>>8,len&255,len>>8);
  }
  if (len>=parms[1]) {
   // we have everything
   sockData[sckt] += line.substr(colon+1,parms[1]);
   return line.substr(colon+parms[1]+1); // return anything else
  } else {
   // still some to get - use getData to request a callback
   sockData[sckt] += line.substr(colon+1,len);
   at.getData(parms[1]-len, function(data) { sockData[sckt] += data; });
   return "";
  }
}
// -----------------------------------------------------------------------------------

function changeMode(callback, err) {
  if (err) return callback(err);
  at.cmd("AT+CWMODE="+wifiMode+"\r\n", 2000, function(cwm) {
    if (cwm!="no change" && cwm!="OK" && cwm!="WIFI DISCONNECT")
      callback("CWMODE failed: "+(cwm?cwm:"Timeout"));
    else
      callback(null);
  });
}

function sckOpen(ln) {
  var sckt = ln[0];
  //console.log("SCKOPEN", JSON.stringify(ln),"current",JSON.stringify(socks[sckt]));
  if (socks[sckt]===undefined && socks[MAXSOCKETS]) {
    // if we have a server and the socket randomly opens, it's a new connection
    socks[sckt] = "Accept";
  } else if (socks[sckt]=="Wait") {
    // everything's good - we're connected
    socks[sckt] = true;
  } else {
    // Otherwise we had an error - timeout? but it's now open. Close it.
    at.cmd('AT+CIPCLOSE='+sckt+'\r\n',1000, function() {
      socks[sckt] = undefined;
    });
  }
}

function sckClosed(ln) {
  //console.log("CLOSED", JSON.stringify(ln));
  socks[ln[0]] = sockData[ln[0]]!="" ? "DataClose" : undefined;
}

function turnOn(mode, callback) {
  var wasOff = wifiMode == 0;
  wifiMode |= mode;
  if (wasOff) {
    at = require("AT").connect(Serial2);
    at.register("+IPD", ipdHandler);
    at.registerLine("0,CONNECT", sckOpen);
    at.registerLine("1,CONNECT", sckOpen);
    at.registerLine("2,CONNECT", sckOpen);
    at.registerLine("3,CONNECT", sckOpen);
    at.registerLine("4,CONNECT", sckOpen);
    at.registerLine("0,CLOSED", sckClosed);
    at.registerLine("1,CLOSED", sckClosed);
    at.registerLine("2,CLOSED", sckClosed);
    at.registerLine("3,CLOSED", sckClosed);
    at.registerLine("4,CLOSED", sckClosed);
    at.registerLine("WIFI CONNECTED", function() { connected |= MODE.CLIENT; exports.emit("associated"); });
    at.registerLine("WIFI GOT IP", function() { exports.emit("connected"); });
    at.registerLine("WIFI DISCONNECTED", function() { connected &= ~MODE.CLIENT; exports.emit("disconnected"); });
    exports.at = at;
    require("NetworkJS").create(netCallbacks);
    at.cmd("\r\nAT+RST\r\n", 10000, function cb(d) {
      if (d=="ready" || d=="Ready.") setTimeout(function() { // wait a little
        at.cmd("ATE0\r\n",1000,function cb(d) { // turn off echo
          if (d=="ATE0") return cb;
          if (d=="OK") {
            at.cmd("AT+CIPDINFO=1\r\n",1000,function(d) { // enable IP Info in +IPD
              if (d!="OK") return callback("CIPDINFO failed: "+(d?d:"Timeout"));
              at.cmd("AT+CIPMUX=1\r\n",1000,function(d) { // turn on multiple sockets
                if (d!="OK") return callback("CIPMUX failed: "+(d?d:"Timeout"));
                at.cmd('AT+UART_CUR=115200,8,1,0,2\r\n',500,function(d) { // enable flow control
                  if (d!="OK") return callback("UART_CUR failed: "+(d?d:"Timeout"));
                  else setTimeout(function() {
                    changeMode(callback);
                  }, 500);
                });
              });
            });
          }
          else callback("ATE0 failed: "+(d?d:"Timeout"));
        });
      }, 500);
      else if (d===undefined) callback("No 'ready' after AT+RST");
      else return cb;
    });

    WIFI_BOOT.set(); // out of boot mode
    Bangle.setWiFiPower(1, "lib");
  } else {
    changeMode(callback);
  }
}
function turnOff(mode, callback) {
  callback = callback||function(){};
  wifiMode &= ~mode;
  if (!wifiMode) {
    Serial2.removeAllListeners();
    at = undefined;
    exports.at = undefined;
    Bangle.setWiFiPower(0, "lib");
    WIFI_BOOT.reset();
    socks = []; // force close of all sockets
    setTimeout(callback,1);
  } else {
    changeMode(callback, null);
  }
}

/** Power on the ESP8266 and connect to the access point
      apName is the name of the access point
      options can contain 'password' which is the AP's password
      callback is called when a connection is made */
exports.connect = function(apName, options, callback) {
  var apKey = "";
  callback = callback||function(){};
  if (options.password!==undefined) apKey=options.password;
  turnOn(MODE.CLIENT, function(err) {
    if (err) return callback(err);
    at.cmd("AT+CWJAP="+JSON.stringify(apName)+","+JSON.stringify(apKey)+"\r\n", 20000, function cb(d) {
      if (["WIFI DISCONNECT","WIFI CONNECTED","WIFI GOT IP","+CWJAP:1"].indexOf(d)>=0) return cb;
      if (d!="OK") setTimeout(callback,0,"WiFi connect failed: "+(d?d:"Timeout"));
      else setTimeout(callback,0,null);
    });
  });
};

/** Disconnect from the WiFi network and power down the
ESP8266. */
exports.disconnect = function(callback) {
  turnOff(MODE.CLIENT, callback);
};

/* Create a WiFi access point allowing stations to connect.
     ssid - the AP's SSID
     options.password - the password - must be at least 8 characters (or 10 if all numbers)
     options.authMode - "open", "wpa2", "wpa", "wpa_wpa2"
     options.channel - the channel of the AP
*/
exports.startAP = function(ssid, options, callback) {
  callback = callback||function(){};
  options = options||{};
  var enc = options.password?3:0; // wpa2 or open
  if (options.authMode) {
    enc={
      "open":0,
      "wpa":2,
      "wpa2":3,
      "wpa_wpa2":4
    }[options.authMode];
    if (enc===undefined) throw new Error("Unknown authMode "+options.authMode);
  }
  if (enc) {
    if (!options.password || options.password.length<8) throw new Error("Password must be at least 8 characters");
  } else options.password="";
  if (options.channel===undefined) options.channel=5;
  turnOn(MODE.AP, function(err) {
    if (err) return callback(err);
    at.cmd("AT+CWSAP="+JSON.stringify(ssid)+","+JSON.stringify(options.password)+","+options.channel+","+enc+"\r\n", 5000, function(cwm) {
      if (cwm!="OK") callback("CWSAP failed: "+(cwm?cwm:"Timeout"));
      else {
        connected |= MODE.AP;
	callback(null);
      }
    });
  });
};

/* Stop being an access point and disable the AP operation mode.
   AP mode can be re-enabled by calling startAP. */
exports.stopAP = function(callback) {
  connected &= ~MODE.AP;
  turnOff(MODE.AP, callback);
};

/* Scan for access points, and call the callback(err, result) with
   an array of {ssid, authMode, rssi, mac, channel} */
exports.scan = function(callback) {
  var aps = [];
  turnOn(MODE.CLIENT, function(err) {
    if (err) return callback(err);
    at.cmdReg("AT+CWLAP\r\n", 5000, "+CWLAP:",
      function(d) {
        var ap = d.slice(8,-1).split(",");
        aps.push({ ssid : JSON.parse(ap[1]),
                   authMode: ENCR_FLAGS[ap[0]],
                   rssi: parseInt(ap[2]),
                   mac : JSON.parse(ap[3]),
                   channel : JSON.parse(ap[4]) });
      },
      function() { callback(null, aps); }
    );
  });
};

/** Get the IP and MAC address when connected to an AP and call
`callback(err, { ip : ..., mac : ...})`. If err isn't null,
it contains a string describing the error. You must be connected to
an access point to be able to call this successfully. For AP mode use getAPIP */
exports.getIP = function(callback) {
  var ip = {};
  at.cmd("AT+CIFSR\r\n", 1000, function cb(d) {
    if (d===undefined) { callback("Timeout"); return; }
    else if (d.substr(0,12)=="+CIFSR:STAIP") ip.ip = d.slice(14,-1);
    else if (d.substr(0,13)=="+CIFSR:STAMAC") ip.mac = d.slice(15,-1);
    else if (d=="OK") { callback(null, ip); return; }
    return cb;
  });
};

/* Set WiFi client IP address. Call with
either: wifi.setIP(undefined, callback) // enable DHCP (the default) - can take a few seconds to complete
either: wifi.setIP({ip:"192.168.1.9"}, callback) // disable DHCP, use static IP
or: wifi.setIP({ip:"192.168.1.9", gw:"192.168.1.1", netmask:"255.255.255.0"}, callback) // disable DHCP, use static IP
You must be connected to an access point to be able to call this successfully
*/
exports.setIP = function(settings, callback) {
  var cmd, timeout;
  if (typeof settings!="object" || !settings.ip) {
    cmd = "AT+CWDHCP_CUR=1,1\r\n";
    timeout = 20000;
  } else {
    var args = [JSON.stringify(settings.ip)];
    if (settings.gw) {
      args.push(JSON.stringify(settings.gw));
      args.push(JSON.stringify(settings.netmask||"255.255.255.0"));
    }
    cmd = "AT+CIPSTA_CUR="+args.join(",")+"\r\n";
    timeout = 3000;
  }
  at.cmd(cmd, timeout, function(d) {
    if (d=="OK") callback(null);
    else return callback("setIP failed: "+(d?d:"Timeout"));
  });
};

/* Calls the callback with {ip,gw,netmask,mac} of the WiFi Access Point.
You must be in AP mode with startAP to get useful values returned. */
exports.getAPIP = function(callback) {
  var ip = {};
  at.cmd("AT+CIPAP_CUR?\r\n", 1000, function cb(d) {
    if (d===undefined) { callback("Timeout"); return; }
    if (d=="OK") {
      at.cmd("AT+CIPAPMAC_CUR?\r\n", 1000, function cbm(d) {
        if (d===undefined) { callback("Timeout"); return; }
        if (d=="OK") { callback(null, ip); return; }
        if (d.substr(0,14)=="+CIPAPMAC_CUR")
          ip.mac = JSON.parse(d.substr(10));
        return cbm;
      });
      return;
    }
    if (d.substr(0,10)=="+CIPAP_CUR") {
      d = d.split(":");
      if (d[1]=="gateway")d[1]="gw";
      ip[d[1]] = JSON.parse(d[2]);
    }
    return cb;
  });
};

/* Set WiFi access point details. Call with
either: wifi.setAPIP({ip:"192.168.1.1"}, callback)
or: wifi.setAPIP({ip:"192.168.1.1", gw:"192.168.1.1", netmask:"255.255.255.0"}, callback)
You must be in AP mode with startAP to be able to call this successfully
*/
exports.setAPIP = function(settings, callback) {
  var args = [JSON.stringify(settings.ip)];
  if (settings.gw) {
    args.push(JSON.stringify(settings.gw));
    args.push(JSON.stringify(settings.netmask||"255.255.255.0"));
  }
  at.cmd("AT+CIPAP_CUR="+args.join(",")+"\r\n", 3000, function(d) {
    if (d=="OK") callback(null);
    else return callback("setAPIP failed: "+(d?d:"Timeout"));
  });
};

/* Set the name of the Espruino WiFi's access point (this isn't DNS, this is just the WiFi access point name) */
exports.setHostname = function(hostname, callback) {
  turnOn(MODE.CLIENT, function(err) {
    if (err) return callback(err);
    at.cmd("AT+CWHOSTNAME="+JSON.stringify(hostname)+"\r\n",500,function(d) {
      callback(d=="OK"?null:d);
    });
  });
};

/* Ping the given address. Callback is called with the ping time
in milliseconds, or undefined if there is an error */
exports.ping = function(addr, callback) {
  var time;
  at.cmd('AT+PING="'+addr+'"\r\n',1000,function cb(d) {
    if (d && d[0]=="+") {
      time=d.substr(1);
      return cb;
    } else if (d=="OK") callback(time); else callback();
  });
};

/** This function returns some of the internal state of the WiFi module, and can be used for debugging */
exports.debug = function() {
  return {
    wifiMode : wifiMode,
    connected : connected,
    socks : socks,
    sockData : sockData
  };
};
