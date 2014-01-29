SPI1.setup({baud:3200000, mosi:A7});
var arr = [255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0];
setInterval("SPI1.send4bit(arr, 0b0001, 0b0011);", 200)


SPI1.setup({baud:3200000, mosi:A7});
SPI1.send4bit([255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0], 0b0001, 0b0011);

SPI1.setup({baud:3200000, mosi:A7});
SPI1.send(1)

var arr = new Uint8Array(75)
n=0;for(i=0;i<25;i++) {
  arr[n++] = i*10;
  arr[n++] = 0;
  arr[n++] = 0;
}
SPI1.send4bit(arr, 0b0001, 0b0011);

function () {
  x++;
  n=0;for(i=0;i<25;i++) {
    arr[n++] = 128+Math.sin(i*0.5+x*0.06)*127;
    arr[n++] = 128+Math.sin(i+x*0.05)*127;
    arr[n++] = 128+Math.sin(i*1.2+x*0.07)*127;
  }
  SPI1.send4bit(arr, 0b0001, 0b0011);
}

// --------------------------------------------------------------------------------------
var arr = new Uint8Array(75);

function () {
  amt += dir;
  if (amt < 0) {
    amt = 0;
  } else if (amt > 1.5 && dir>0) {
    amt = 3;
    dir = 0;
  } else {
    if (!BTN1.read()) dir = -0.01;
  }
  if (amt<1 && BTN1.read()) dir = 0.04;


  var n = 0;
   for(var i=0;i<25;i++) {
    var c = 255*(2*amt - (Math.abs(i-12.5)/12.5 +0.5));
    arr[n++] = E.clip(256+c*2, 0, 255);
    arr[n++] = E.clip(64+c, 0, 255);
    arr[n++] = E.clip(c/2, 0, 255);
  }
  SPI1.send4bit(arr, 0b0001, 0b0011);
}


var amt = 22.5;
var stepInterval = 1;
function onInit() {
  stepInterval = setInterval(step, 100);
}
var dir = 0.1;

SPI1.setup({baud:3200000, mosi:A7});
setInterval(step, 50);

