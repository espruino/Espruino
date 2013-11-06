function onInit() {
  SPI1.setup({baud:3200000, mosi:A7});
  C3.set(); // Pull the light sensor's potential divider up to 3.3v
}
onInit();

var light = 0.0; // an average

function getPattern() {
  var lightInstant = analogRead(C1)*3;
  light = lightInstant*0.1 + light*0.9;
  var cols = [];
  for (var i=0;i<50;i++) {
     var c = (-Math.abs(i-25)*10) + light*1024 - 200;
     if (c<0) c=0;
     if (c>255) c=255;
     cols.push(c);
     c = (-Math.abs(i-25)*10) + light*1024 - 450;
     if (c<0) c=0;
     if (c>255) c=255;
     cols.push(c);
     c = (-Math.abs(i-25)*10) + light*1024 - 600;
     if (c<0) c=0;
     if (c>255) c=255;
     cols.push(c);
  }
  return cols;
}

function doLights() {
  SPI1.send4bit(getPattern(), 0b0001, 0b0011);
}

setInterval(doLights, 50);















SPI1.send4bit([255,0,0], 0b0001, 0b0011);

SPI1.send4bit([0,255,0], 0b0001, 0b0011); // green

SPI1.send4bit([0,0,255], 0b0001, 0b0011); // blue

SPI1.send4bit([255,255,255], 0b0001, 0b0011); // white

function getPattern() {
  var cols = [];
  for (var i=0;i<50;i++) {
     cols.push(i*5);
     cols.push(i*5);
     cols.push(i*5);
  }
  return cols;
}



function doLights() {
  SPI1.send4bit(getPattern(), 0b0001, 0b0011);
}

doLights();




This is a set of lights that smoothly changes colour and pattern depending on the amount of light in the room.<br />
<br />
Apologies for the video - you'll need to view it in HD, and even then you may not be able top make out all of the code.<br />
<br />
You'll need:
<ul>
	<li>
		An <a href="http://www.st.com/web/en/catalog/tools/PF252419#">STM32F4DISCOVERY</a> board</li>
	<li>
		A <a href="http://www.ebay.co.uk/sch/i.html?_nkw=USB+TTL">USB-TTL converter </a>- this is not vital, without it the LEDs will flicker when USB is plugged in, but once you remove it they will be fine</li>
	<li>
		A <a href="http://www.ebay.co.uk/sch/i.html?_nkw=WS2811">WS2811</a> LED string</li>
	<li>
		A Light Dependent Resistor (LDR) and matching normal resistor (in my case 200k)</li>
	<li>
		The <a href="http://www.espruino.com">Espruino JavaScript interpreter</a> Software</li>
</ul>
<br />
I connected:
<ul>
	<li>
		The white+red wires of the WS2811s to 0v and 5v</li>
	<li>
		The green wire of the LDR to pin PA7</li>
	<li>
		The LDR between ground and pin PC1</li>
	<li>
		A 200kOhm resistor between PC3 and PC1</li>
</ul>
<br />
There's more information on controlling and wiring up the lights on the <a href="http://www.espruino.com/Tutorial+5">Espruino tutorial for WS2811s</a>. The actual code you need to copy and paste in is:<br />
<blockquote>
	<code>function onInit() {<br />
	&nbsp; SPI1.setup({baud:3200000, mosi:A7});<br />
	&nbsp; C3.set(); // Pull the light sensor's potential divider up to 3.3v<br />
	}<br />
	onInit();<br />
	<br />
	var light = 0.0; // an average<br />
	<br />
	function getPattern() {&nbsp;&nbsp;&nbsp;<br />
	&nbsp; var lightInstant = analogRead(C1)*3;<br />
	&nbsp; light = lightInstant*0.1 + light*0.9;<br />
	&nbsp; var cols = [];<br />
	&nbsp; for (var i=0;i&lt;50;i++) {<br />
	&nbsp;&nbsp;&nbsp;&nbsp; var c = (-Math.abs(i-25)*10) + light*1024 - 200;<br />
	&nbsp;&nbsp;&nbsp;&nbsp; if (c&lt;0) c=0;<br />
	&nbsp;&nbsp;&nbsp;&nbsp; if (c&gt;255) c=255;<br />
	&nbsp;&nbsp;&nbsp;&nbsp; cols.push(c);<br />
	&nbsp;&nbsp;&nbsp;&nbsp; c = (-Math.abs(i-25)*10) + light*1024 - 450;<br />
	&nbsp;&nbsp;&nbsp;&nbsp; if (c&lt;0) c=0;<br />
	&nbsp;&nbsp;&nbsp;&nbsp; if (c&gt;255) c=255;<br />
	&nbsp;&nbsp;&nbsp;&nbsp; cols.push(c);<br />
	&nbsp;&nbsp;&nbsp;&nbsp; c = (-Math.abs(i-25)*10) + light*1024 - 600;<br />
	&nbsp;&nbsp;&nbsp;&nbsp; if (c&lt;0) c=0;<br />
	&nbsp;&nbsp;&nbsp;&nbsp; if (c&gt;255) c=255;<br />
	&nbsp;&nbsp;&nbsp;&nbsp; cols.push(c);<br />
	&nbsp; }<br />
	&nbsp; return cols;<br />
	}<br />
	<br />
	function doLights() {&nbsp;&nbsp;&nbsp;<br />
	&nbsp; SPI1.send4bit(getPattern(), 0b0001, 0b0011);<br />
	}<br />
	<br />
	setInterval(doLights, 50);</code></blockquote>
<br />
And job done! If you type 'save()' it'll keep working even after power off.
