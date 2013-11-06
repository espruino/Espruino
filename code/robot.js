clearWatch();
clearInterval();

var TRIG = A0; // ultrasonic trigger
var ECHO = A1; // ultrasonic echo signal
var MOTORS = [B0,B1,A7,A6]; // pins for the motors

var t1 = 0;
var dist = 0;
var inManouver = false;

// Get the distance from the sensor
setWatch(function(e) { t1=e.time; }, ECHO, { repeat:true, edge:'rising'  });
setWatch(function(e) { var dt=e.time-t1; dist = (dt*1000000)/57.0; },  ECHO, { repeat:true, edge:'falling' });
// 20 times a second, trigger the distance sensor
setInterval("digitalPulse(TRIG,1, 10/1000.0)",50);

// reverse, turn, and go forwards again
function backup() {
  inManouver = true;
  digitalWrite(MOTORS, 5); // back
  setTimeout(function() {
    digitalWrite(MOTORS, 6); // turn
      setTimeout(function() {
        inManouver = false;
        digitalWrite(MOTORS, 6); // forward again
      }, 500);
  }, 500);
}
function forward() {
   digitalWrite(MOTORS, 10);
}
function stop() {
   digitalWrite(MOTORS, 0);
}
function onInit() {
  forward();
}
function step() {
  // if we detect we're getting too close, turn around
  if (dist < 20 && !inManouver)
    backup();
}

setInterval(step, 100); // check every 100ms to see if we're too close
onInit(); // start going forwards





var TRIG = A0;
var ECHO = A1;
var MOTORS = [B0,B1,A7,A6];
var t1 = 0.91688;
var dist = 76136.251949;
var inManouver = false;
var targetDist = 26.327729;
function backup() {
  inManouver = true;
  digitalWrite(MOTORS, 5); // back
  digitalWrite([LED1,LED2,LED3],4);
  setTimeout(function() {
    digitalWrite(MOTORS, 6); // turn
    digitalWrite([LED1,LED2,LED3],5);
      setTimeout(function() {
        inManouver = false;
        forward();
      }, 500);
  }, 500);
}
function forward() {
   digitalWrite(MOTORS, 10);
   digitalWrite([LED1,LED2,LED3],1);
}
function stop() {
   digitalWrite(MOTORS, 0);
}
function onInit() {
  Serial3.setConsole();
  forward();
}
function step() {
  if (mode==0) {
    if (dist < 20 && !inManouver)
      backup();
  }
  if (mode==1) {
    var d = targetDist - dist;
    var c = 5;
//    print(d);
    digitalWrite(MOTORS[0], d<-c);
    digitalWrite(MOTORS[1], d>c);
    digitalWrite(MOTORS[2], d<-c);
    digitalWrite(MOTORS[3], d>c);
  }
}
var led = undefined;
var mode = 1;
function onButton() {
  mode++;
  if (mode>1) mode=0;
  digitalWrite([LED1,LED2,LED3], 1+mode);
  if (mode == 0) forward();
  if (mode == 1) targetDist = dist;
}
setInterval("digitalPulse(TRIG,1, 10/1000.0)", 50);
setInterval(step, 100);
setWatch(function (e) { t1=e.time; }, A1, { repeat:true, edge:'rising' });
setWatch(function (e) { var dt=e.time-t1; dist = (dt*1000000)/57.0; }, A1, { repeat:true, edge:'falling' });
setWatch(onButton, A3, { repeat:true, edge:'rising' });










// simple bluetooth robot control
// Pins for the motors
var MOTORS = [B0,B1,A7,A6];
// Pin values to set
var GO = { FORWARD: 0b1010, BACK : 0b0101, LEFT : 0b0110, RIGHT : 0b1001 };

function move(motorState, time) {
   digitalWrite(MOTORS, motorState);
   setTimeout("digitalWrite(MOTORS, 0);", 500);
}

Serial3.setup(9600);
Serial3.onData(function(e) {
  var command = e.data;
  if (command=="w") move(GO.FORWARD, 500);
  if (command=="s") move(GO.BACK, 500);
  if (command=="a") move(GO.LEFT, 500);
  if (command=="d") move(GO.RIGHT, 500);
});


// voice bluetooth robot control
// Pins for the motors
var MOTORS = [B0,B1,A7,A6];
// Pin values to set
var GO = { FORWARD: 0b1010, BACK : 0b0101, LEFT : 0b0110, RIGHT : 0b1001 };

function move(motorState, time) {
   digitalWrite(MOTORS, motorState);
   setTimeout("digitalWrite(MOTORS, 0);", 500);
}

Serial3.setup(9600);
var command = "";
Serial3.onData(function(e) {
  command += e.data;
  if (e.data==" " || e.data=="\n") {
    command="";
  } else {
    print(command);
    if (command=="forward") move(GO.FORWARD, 500);
    if (command=="back") move(GO.BACK, 500);
    if (command=="left") move(GO.LEFT, 500);
    if (command=="right") move(GO.RIGHT, 500);
  }
});

