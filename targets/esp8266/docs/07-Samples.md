This file will contain tested and richer samples of using Espruino on the ESP8266.

##An HTTP responder
The following script will setup the device to be an HTTP listener.  Connect a browser to:

1. `http://<ESP_IP>/hello`
2. `http://<ESP_IP>/goodbye`
3. `http://<ESP_IP>/nothing`

Here is the script to run.

	ESP8266WiFi.init();
	var http = require("http");
	var httpSrv = http.createServer(function(request, response) {
		response.write("<html><body>");
		if (request.url == "/hello") {
			response.write("<b>Welcome</b> to the ESP8266 test.");
		} else if (request.url == "/goodbye") {
			response.write("<b>Please</b> come back again soon.");
		} else {
			response.write("Sorry ... I didn't understand!");
		}
		response.end("</body></html>");
	});
	httpSrv.listen(80);
	
##An HTTP GET request
The following is a simple HTTP GET request:

	ESP8266WiFi.init();
	var http = require("http");
	http.get({
		host: "184.168.192.49",
		port: 80,
		path: "/"
	}, function(response) {
		print("get callback!");
	    response.on('data', function(data) {
	      print("We got data: " + data);
	    });
	    response.on('close', function() {
	       print("The response connection closed");
	    });
	});