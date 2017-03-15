// Start being an access point.
// Start being a web server.
//   Listen on:
//   - /list - Return a list of access points
//   - /select?ssid=<SSID>&password=<password> - Select an SSID and supply a password.
// When a request arrives from a browser, build a page which contains the
// list of access points to which we can connect.
// When the user selects an access point with password, connect to that access point.


// 2. Configure a Web Server.
function startWebServer() {
	var http = require("http");
	var httpServer = http.createServer(function(request, response) {
		console.log("Received a browser request looking for " + request.url);
		// Handle browser requests here ...
		var parsedURL = url.parse(request.url, true);
		if (parsedURL.pathname == "/list") {
			// Process the list request here ...
			// Handle the list request
			response.writeHead(200);
			response.end("process /list");
			return;
		} else if (parsedURL.pathname == "/select") {
			// Handle the select request
			if (!parsedURL.query.hasOwnProperty("ssid") || !parsedURL.query.hasOwnProperty("password")) {
				console.log("No ssid or no password in /select request: " + JSON.stringify(parsedURL));
				response.writeHead(200);
				response.end("Hello World"); 
				return;
			}
			// Process the select request here...
			response.writeHead(200);
			response.end("Process /select"); 
			return;
		}
		// Unknown service
		response.writeHead(200);
		response.end("Hello World"); 
	});
	httpServer.listen(80);
	console.log("Web Server now started ... listening on port 80");
}
var wifi = require("Wifi");

// 1. Be an access point

wifi.startAP("MYESP32", {
  "authMode": "open"
}, function(err) {
  console.log("AP now started: " + err);
  startWebServer();
});



//Connect to an access point and be a simple web server
//
/*
var ssid="guest";
var password="kolbanpassword";

wifi.connect(ssid, {password: password}, function() {
	startWebServer();
});
*/