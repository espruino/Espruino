clearWatch();

var pressed = false;

setWatch(function() {
if(!pressed)
  doColour(red);
  pressed = true;
}, C6, { repeat: true, edge:'rising'});

setWatch(function() {
  if(!pressed)
doColour(green);
    pressed = true;
}, C7, { repeat: true, edge:'rising'});

setWatch(function() {
  if(!pressed)
doColour(yellow);
    pressed = true;
}, C9, { repeat: true, edge:'rising'});

setWatch(function() {
  if(!pressed)
doColour(blue);
    pressed = true;
}, C8, { repeat: true, edge:'rising'});
function onInit() {
  pinMode(C6, "input_pulldown");
  pinMode(C7, "input_pulldown");
  pinMode(C8, "input_pulldown");
  pinMode(C9, "input_pulldown");
}
onInit();

setWatch(function() {
  pressed = false;
clearInterval();
setInterval(doLights, 20);
}, BTN, { repeat: true, edge:'rising'});


function blue() {
  var cols = [];
  for (var i=0;i<25;i++) {
     cols.push(0);
     cols.push(0);
     cols.push(255);
  }
  return cols;
}

function red() {
  var cols = [];
  for (var i=0;i<25;i++) {
     cols.push(255);
     cols.push(0);
     cols.push(0);
  }
  return cols;
}

function green() {
  var cols = [];
  for (var i=0;i<25;i++) {
     cols.push(0);
     cols.push(255);
     cols.push(0);
  }
  return cols;
}

function yellow() {
  var cols = [];
  for (var i=0;i<25;i++) {
     cols.push(255);
     cols.push(255);
     cols.push(0);
  }
  return cols;
}

function getPattern() {
  pos++;
  var cols = [];
  for (var i=0;i<25;i++) {
     var col = Math.round((Math.sin(i+pos)+1) * 127);
     cols.push(col);
     col = Math.round((Math.sin(i+pos*1.321124)+1) * 127);
     cols.push(col);
     cols.push(col);
  }
  return cols;
}

function doLights() {
  SPI1.send4bit(getPattern(), 0b0001, 0b0011);
}

function doColour(col) {
  SPI1.send4bit(col(), 0b0001, 0b0011);
  clearInterval();
}

var pos = 3668;
clearInterval();
setInterval(doLights, 20);
SPI1.setup({"baud":3200000,"mosi":A7});

