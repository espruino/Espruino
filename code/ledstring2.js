arr = new Uint8Array(26*3);
function step() {
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
var amt = 0;
var stepInterval = 3;
function onInit() {
  stepInterval = setInterval(step, 100);
}
var dir = -0.01;
var d = undefined;
setInterval(step, 20);
SPI1.setup({"baud":3200000,"mosi":A7});



>for (i=0;i<25;i++) r[i] = Math.random();

>var r = new Float32Array(25);
>step = function () {
:  amt += 0.3;
:
:  var n = 0;
:   for(var i=0;i<25;i++) {
:    var c = 255*(Math.sin(r[i]*220+amt) +0.5);
:    arr[n++] = E.clip(256+c*2, 0, 255);
:    arr[n++] = E.clip(64+c, 0, 255);
:    arr[n++] = E.clip(c/2, 0, 255);
:  }
:  SPI1.send4bit(arr, 0b0001, 0b0011);
:};

var cols = [new Uint8Array(4),[0,0,255,255],[0,0,0,0]];
function setCol(n,r,g,b) {
  cols[0][n] = r*255;
  cols[1][n] = g*255;
  cols[2][n] = b*255;
}

// christmas
function () {
  amt += 0.3;

  var n = 0;
   for(var i=0;i<25;i++) {
    var c = 255*(Math.sin(r[i]*220+amt)*0.5 +0.5);
    arr[n++] = E.clip(c*2, 0, 255);
    arr[n++] = E.clip(c*2-256, 0, 255);
    arr[n++] = 0;
  }
  SPI1.send4bit(arr, 0b0001, 0b0011);
}

// wedding
=function () {
  amt += 0.1;

  var n = 0;
   for(var i=0;i<25;i++) {
    var c = Math.sin(i*0.2+amt)*0.5+0.5;
    arr[n++] = E.clip(64+c*128, 0, 255);
    arr[n++] = 0;
    arr[n++] = E.clip(255-c*128,0,255);
  }
  SPI1.send4bit(arr, 0b0001, 0b0011);
}

// patriotic
=function () {
  amt += 0.15;

  var n = 0;
   for(var i=0;i<25;i++) {
    var c = (Math.random()*3)|0;
    arr[n++] = c<2 ? 255 : 0;
    arr[n++] = c==1 ? 255 : 0;
    arr[n++] = c>0 ? 255 : 0;
  }
  SPI1.send4bit(arr, 0b0001, 0b0011);
}


// One end to other
function () {
  amt += 0.1;
  var pos = (Math.sin(amt)*0.5+0.5)*25;
  var n = 0;
   for(var i=0;i<25;i++) {
    var c = Math.abs(i-pos);
    arr[n++] = E.clip((4-c)*63,0,255);
    arr[n++] = 0;
    arr[n++] = 0;
  }
  SPI1.send4bit(arr, 0b0001, 0b0011);
}

