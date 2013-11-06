clearWatch();
clearInterval();

var TRIG = A12;
var ECHO = A8;

var x=0;
var t1 = 0;
function onStart(e) {
  t1=e.time;
}

function onStop(e) {
  var dt=e.time-t1;
  var dist = (dt*1000000)/57.0;
  //print("dist="+dist);
  //analogWrite(LED1, dist/100);
  x++;
  if (x>LCD.WIDTH) {
    LCD.clear();
    x=0;
  }
  LCD.setPixel(x,dist,0xFFFF);
}

setWatch(onStart, ECHO, { repeat:true, edge:'rising'  });
setWatch(onStop,  ECHO, { repeat:true, edge:'falling' });

setInterval("digitalPulse(TRIG,1, 10/1000.0)",50);
