var servo = [0.75,0.730152,0.717071];
function servoTimer() {
  digitalPulse(B12,1,1+E.clip(servo[0],0,1));
  digitalPulse(B13,1,1+E.clip(servo[1],0,1));
  digitalPulse(B14,1,1+E.clip(servo[2],0,1));
}
function moveTo(x,y) {
//  print(x+","+y);return;
  var d = Math.sqrt(x*x+y*y);
  var s1 = 2*Math.asin(d/2);
  var a = (180-s1)/2;
  var b = Math.atan(x / y);
  var s2 = 90-(a+b);
 // print(s1+","+s2);
  servo[1] = 0.5 - ((s1-(Math.PI*0.5)) / Math.PI)*0.7;
  servo[2] = 0.7 - 0.7*(s2 / Math.PI);
}
var pos = 3.82;
function step() {
  pos = pos+0.02;
  if (pos>4) pos=0;
  var i = pos|0;
  var f = pos-i;
  if (i==0) moveTo(0, 0.5 + f);
  if (i==1) moveTo(f, 1.5);
  if (i==2) moveTo(1, 1.5-f);
  if (i==3) moveTo(1-f, 0.5);
}
setInterval(servoTimer, 50);
setInterval(step, 50);

// --------------------------------------------------


var servo = [0.9,0.505683,0.57334];
function servoTimer() {
  digitalPulse(B12,1,1+E.clip(servo[0],0,1));
  digitalPulse(B13,1,1+E.clip(servo[1],0,1));
  digitalPulse(B14,1,1+E.clip(servo[2],0,1));
}
function moveTo(x,y) {
//  print(x+","+y);return;
  var d = Math.sqrt(x*x+y*y);
  var s1 = 2*Math.asin(d/2);
  var a = (180-s1)/2;
  var b = Math.atan(x / y);
  var s2 = 90-(a+b);
 // print(s1+","+s2);
  servo[1] = 0.5 - ((s1-(Math.PI*0.5)) / Math.PI)*0.7;
  servo[2] = 0.7 - 0.7*(s2 / Math.PI);
}
var pos = 1;
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

 f = [0.5 + 2*(x*2.5/z), 1.0 + 2*(y*2.5/z)];
 moveTo(f[0], f[1]);
}
function func(dx,dy) {
 var r = 30*Math.sqrt(dx*dx + dy*dy) + 0.001;
 return Math.sin(r)/r;
}
var f = [0.283096,1.367057];
setInterval(servoTimer, 20);
setInterval(step, 20);
setWatch("pos=0", BTN, { repeat: true });
