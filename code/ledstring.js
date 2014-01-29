// Can't run on the VL board - as it has to be quite fast
// Also needs USB to be unplugged, as servicing the USB messes it up

C5.reset();


SPI1.setup({baud:1600000});

function handle(data) {
  var l = data.length;
  var res = "";
  var chs = "\x11\x13\x31\x33";
  for (var i=0;i<l;i++) {
    var c = Integer.valueOf(data.charAt(i));
    res = res + chs.charAt((c>>6)&3) + chs.charAt((c>>4)&3)  + chs.charAt((c>>2)&3) + chs.charAt(c&3);
  }
  return res;
}

function p(data) {
  var l = data.length;
  for (var i=0;i<l;i++) {
    print(Integer.valueOf(data.charAt(i)));
  }
}
p(handle("\0\0\xFF\0\xFF\0\xFF\0\0"));
SPI1.send(handle("\0\0\xFF\0\xFF\0\xFF\0\0"));

var pos = 0;
function timer() {
 pos++;
 if (pos>49) pos = 0;
 var cols = "";
 for (var i=0;i<50;i++) {
  if (i==pos)
    cols += "\xFF\x7F\x20";
  else
    cols += "\x10\x05\0";
 }
 SPI1.send(handle(cols));
}
setInterval(timer,100);








function testspi() {
 SPI1.setup({baud:3200000});
 var cols = "";
 for (var i=0;i<50;i++) {
    cols += "\xFF\x7F\x20";
 }
 SPI1.send(cols);
}





SPI1.setup({baud:3200000});
var pos = 0;
function timer() {
 pos++;
 if (pos>49) pos = 0;
 var cols = "";
 for (var i=0;i<50;i++) {
  if (i==pos)
    cols += "\xFF\x7F\x20";
  else
    cols += "\x10\x05\0";
 }
 SPI1.send4bit(cols, 0x1, 0x3);
}
setInterval(timer,50);


C5.reset();
SPI1.setup({baud:3200000});
// SPI1.setup({baud:3200000,mosi:D11});D10.reset();
var pos = 0;
function timer() {
 pos++;
 var cols = "";
 for (var i=0;i<50;i++) {
    cols += "\0\0" + String.fromCharCode((1+Math.sin((i+pos)*0.1))*127);
 }
 SPI1.send4bit(cols, 0x1, 0x3);
}
setInterval(timer,50);


function timer() {
 pos++;
 var cols = "";
 for (var i=0;i<50;i++) {
    cols += String.fromCharCode((1+Math.sin((i+pos)*0.1324))*127) + String.fromCharCode((1+Math.sin((i+pos)*0.1654))*127) + String.fromCharCode((1+Math.sin((i+pos)*0.1))*127);
 }
 SPI1.send4bit(cols, 0x1, 0x3);
}

SPI1.send4bit([255,0,0], 0b0001, 0b0011);



function timer() {
  pos++;
  var cols = [];
  for (var i=0;i<50;i++) {
     cols.push(1+Math.sin((i+pos)*0.1324)*127);
     cols.push(1+Math.sin((i+pos)*0.1654)*127);
     cols.push(1+Math.sin((i+pos)*0.1)*127);
  }
  SPI1.send4bit(cols, 0x1, 0x3);
 }


function getPattern() {
  var cols = [];
  for (var i=0;i<50;i++) {
     cols.push(i*5);
     cols.push(i*5);
     cols.push(i*5);
  }
  return cols;
}

var pos = 0;
function getPattern() {
  pos++;
  var cols = [];
  for (var i=0;i<50;i++) {
     cols.push(Math.round((1+Math.sin((i+pos)*0.1324))*127));
     cols.push(Math.round((1+Math.sin((i+pos)*0.1654))*127));
     cols.push(Math.round((1+Math.sin((i+pos)*0.1))*127));
  }
  return cols;
}


function doLights() {
  SPI1.send4bit(getPattern(), 0b0001, 0b0011);
}

var pos = 0;
function getPattern() {
  pos++;
  var cols = "";
  for (var i=0;i<50;i++) {
     cols += String.fromCharCode((1 + Math.sin((i+pos)*0.1324)) * 127) +
             String.fromCharCode((1 + Math.sin((i+pos)*0.1654)) * 127) +
             String.fromCharCode((1 + Math.sin((i+pos)*0.1)) * 127);
  }
  return cols;
}

function getPattern() {
  pos++;
  var cols = "";
  for (var i=0;i<50;i++) {
     cols += String.fromCharCode(0) +
             String.fromCharCode(0) +
             String.fromCharCode( Math.random()*255 );
  }
  return cols;
}



// haloween
function () {
  amt += 0.05;
  var n = 0;
   for(var i=0;i<25;i++) {
    arr[n++] = 0;
    arr[n++] = E.clip(90*Math.sin(0.5*i+amt) + 128 + 90*Math.sin((0.3*i-amt)*3.2324), 0, 255);
    arr[n++] = 0;
  }
  SPI1.send4bit(arr, 0b0001, 0b0011);
}

