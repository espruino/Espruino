

while (true)
  LCD.fillRect(Math.random()*320,Math.random()*240,Math.random()*320,Math.random()*240,Math.random()*0xFFFF);


function Touch(callback) {
  SPI1.send([0x90,0],B7); // just wake the controller up

  var watchFunc = function() {
    if (!digitalRead(B6)) {
      clearWatch(watch); // clear this watch
      // scan and call the callback
      var dx = SPI1.send([0x90,0,0],B7);
      var dy = SPI1.send([0xD0,0,0],B7);
      callback((dx[1]*256+dx[2])*LCD.WIDTH/0x8000, 239-(dy[1]*256+dy[2])*LCD.HEIGHT/0x8000);
      // re-add this watch
      watch = setWatch(watchFunc, B6, true);
    }
  };
  var watch = setWatch(watchFunc, B6, true);
}
function onInit() {
  touch = new Touch(function(x,y) { LCD.fillRect(x-8, y-8, x+8, y+8, 0xFFFF); });
}
onInit();


var x = 320;
function newGraphElement() {
  x++;
  if (x>319) {
    x=0;
    LCD.clear();
  }
  var val = analogRead(C0);
  var y = val*240;
  LCD.setPixel(x, y, LCD.col(1,0,0));
  val = analogRead(C1);
  y = val*240;
  LCD.setPixel(x, y, LCD.col(0,1,0));
  LCD.drawString(val+"              ", 0,0,0,0xFFFF);
};
setInterval(newGraphElement, 20);




function onInit() {
  SPI1.send([0x90,0],B7); // just wake the controller up
}
function touchFunc() {
  if (!digitalRead(B6)) { // touch down
    var dx = SPI1.send([0x90,0,0],B7);
    var dy = SPI1.send([0xD0,0,0],B7);
    touchCallback((dx[1]*256+dx[2])*LCD.WIDTH/0x8000, 239-(dy[1]*256+dy[2])*LCD.HEIGHT/0x8000);
  }
};
function touchCallback(x,y) {
  LCD.fillRect(x-5,y-5,x+5,y+5,0xFFFF);
}
onInit();
setInterval(touchFunc, 100);



LCD.fillCircle = function(x,y,rad,col) {
  var pts = parseInt(rad)/2;
  var a = [];
  for (var i=0;i<pts;i++) {
    var t = 2*i*Math.PI/pts;
    a.push(x+Math.sin(t)*rad);
    a.push(y+Math.cos(t)*rad);
  }
  LCD.fillPoly(a,col);
}


LCD.clear();
while (true)
  LCD.drawLine(Math.random()*LCD.WIDTH,Math.random()*LCD.HEIGHT,Math.random()*LCD.WIDTH,Math.random()*LCD.HEIGHT,0xFFFF*Math.random())


// ----------------------------------------------------------- Thermometer
var LCD = function () {};
var onInit = function () {
  drawScreen();
  SPI1.send([0x90,0],B7); // just wake the controller up
};
var touchFunc = function () {
  if (!digitalRead(B6)) { // touch down
    var dx = SPI1.send([0x90,0,0],B7);
    var dy = SPI1.send([0xD0,0,0],B7);
    var pos = [(dx[1]*256+dx[2])*LCD.WIDTH/0x8000, 239-(dy[1]*256+dy[2])*LCD.HEIGHT/0x8000];
    touchCallback(pos[0], pos[1]);
    lastPos = pos;
  } else lastPos = null;
};
var touchCallback = function (x,y) {
  if (lastPos!=null) {
    temp += 0.05 * (lastPos[1]-y);
    if (temp<0) temp=0;
    if (temp>40) temp=40;
  }
  drawScreen();
};
var temp = 10.23;
var drawScreen = function () {
  LCD.clear();
  var tempStr = Math.round(temp*10)/10.0;
  LCD.drawVectorString(tempStr,0,0,100,0xFFFF);
  LCD.drawVectorString("o", 260,0, 25, 0xFFFF);
  LCD.drawVectorString("C", 280,0, 50, 0xFFFF);
  for (var i=0;i<history.length;i++)
    LCD.fillRect(i*LCD.WIDTH*1.0/history.length, getScreenHeightForTemp(history[i]), (i+1)*LCD.WIDTH*1.0/history.length, getScreenHeightForTemp(0), LCD.col(1,0,0));
  LCD.fillRect(0, getScreenHeightForTemp(temp), LCD.WIDTH-1, getScreenHeightForTemp(temp), LCD.col(0,1,0));
};
var lastPos = null;
var Math = function () {};
var history = new Array(30);
var measure = function () {
  var value = analogRead(C0)*40;
  for (var i=0;i<history.length-1;i++) history[i]=history[i+1];
  history[history.length-1] = value;
  digitalWrite(LED1, temp > value);
  drawScreen();
};
var getScreenHeightForTemp = function (t) { return LCD.HEIGHT-(1+t*4); };
setInterval(touchFunc, 100);
setInterval(measure, 2000);

