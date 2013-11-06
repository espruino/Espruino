
function setServo(pin,pos) {
 if (pos<0) pos=0;
 if (pos>1) pos=1;
 analogWrite(pin, (1+pos) / 50.0, {freq:20});
}
setServo(C7, 0.5); // set servo on pin C7 to mid position
// Servo will continue to be controlled without writing any other code


Or just to show off, you can also overload the 'Pin' object to add a new method to it:

Pin.prototype.servo = function(pos) {
 if (pos<0) pos=0;
 if (pos>1) pos=1;
 analogWrite(this, (1+pos) / 50.0, {freq:20});
}
C7.servo(0.5);




// -------------------------------------------------------- Laser tag
var lastPos = undefined;
function touchCallback(x,y) {
  if (lastPos!=undefined && x==undefined) {
    lastPos = undefined;
  } else if (lastPos==undefined && x!=undefined) {
    LCD.clear();
    positions = [];
  }
  if (x!=undefined) {
    if (lastPos!=undefined)
      LCD.drawLine(x,y,lastPos.x,lastPos.y,0xFFFF);
    var pos = {x:x,y:y};
    positions.push([ 0.75 - x*0.5 / LCD.WIDTH, 0.75 - y*0.5 / LCD.HEIGHT ]);
    lastPos = pos;
  }
}

function onInit() {
  SPI1.send([0x90,0],B7); // just wake the controller up
}
function touchFunc() {
  if (!digitalRead(B6)) { // touch down
    var dx = SPI1.send([0x90,0,0],B7);
    var dy = SPI1.send([0xD0,0,0],B7);
    touchCallback((dx[1]*256+dx[2])*LCD.WIDTH/0x8000, 239-(dy[1]*256+dy[2])*LCD.HEIGHT/0x8000);
  } else
    touchCallback();
};
onInit();
setInterval(touchFunc, 50);

var servoPos = [0.5,0.5];
function servoFunc() {
   digitalPulse(D2, 1, 1+servoPos[0]);
   digitalPulse(C11, 1, 1+servoPos[1]);
}
setInterval(servoFunc, 50);

var positions = [];
function servoMove() {
  if (positions.length==0) return;
  pos = (pos + 1) % positions.length;
  servoPos = positions[pos];
}
setInterval(servoMove, 100);





// Sin r/r


function moveTo(x,y) {
  LCD.fillRect(x*200,y*200, x*200+2, y*200+2, 0xFFFF);
}
function func(x,y) {
 var dx = x-0.5;
 var dy = y-0.5;
 var r = 30*Math.sqrt(dx*dx + dy*dy) + 0.001;
 return Math.sin(r)/r;
}
function step() {
 // do a zig-zag motion over x and y
 pos = pos + 0.001;
 if (pos>=1) pos=0;
 var bars = 20.0;
 var p = pos*bars;
 var y = Math.floor(p);
 var x = p-y;
 if (y&1 == 1) x=1-x;
 y = y/bars;
 // work out z
 z = func(x,y);
 y = y - z*0.2;
 moveTo(0.1+x*0.8,0.1+y*0.8);
}

var pos = 0;
setInterval(step, 50);
// when button1 is pressed, clear screen and start again
setWatch("LCD.clear();pos=0;", BTN1, { edge:"falling", repeat: true });


function moveTo(x,y) {
  digitalPulse(D2, 1, 1.25+x*0.5);
  digitalPulse(C11, 1, 1.25+y*0.5);
  x = x * 240;                      ;
  y =239 - (240*y);                 ocessing - clearing all timers and watches.
  LCD.fillRect(x,y,x+1,y+1, 0xFFFF);
};

// super 3d sin r/r
function func(dx,dy) {
 var r = 30*Math.sqrt(dx*dx + dy*dy) + 0.001;
 return Math.sin(r)/r;
};
function step() {
 // do a zig-zag motion over x and y
 pos = pos + 0.0005;
 if (pos>=1) pos=1;
 var bars = 20.0;
 var p = pos*bars;
 var y = Math.floor(p);
 var x = p-y;
 if (y&1 == 1) x=1-x;
 x = x-0.5;
 var z = y/bars - 0.5;
 // work out y, x and z are between -0.5 and 0.5
 y = func(x,z)*0.4;

 var ry = 0.5;
 var rx = 0.5;
 var xt = Math.cos(ry)*x + Math.sin(ry)*z;
 z = Math.cos(ry)*z - Math.sin(ry)*x; x=xt;
 var yt = Math.cos(rx)*y + Math.sin(rx)*z;
 z = Math.cos(rx)*z - Math.sin(rx)*y; y=yt;

 z += 4;

 moveTo(0.5 + (x*2.5/z), 0.5 + (y*2.5/z));
};

