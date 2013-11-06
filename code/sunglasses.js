var moveServo = function (p) {
 var pulses = 20;
 var f = function() { digitalPulse(D0,1,p); if (pulses-->0) setTimeout(f, 50); };
 f();
};
var on = 0;
var setOn = function () {
  on=true;
  digitalWrite(LED1,1);

  setTimeout("digitalWrite(LED1,0)",1000);
  moveServo(2);
};
var setOff = function () {
  on=false;
  digitalWrite(LED2,1);

  setTimeout("digitalWrite(LED2,0)",1000);
  moveServo(1);
};
var test = function () {
 digitalWrite(A0,0);
 digitalWrite(A2,1);
 average=average*0.8 + 0.2*analogRead(A1);
 if (!on && average<0.4) setOn();
 if (on && average>0.5) setOff();
};

var average = 0;
var blink = function () {
 digitalWrite(LED1,1);
 setTimeout('digitalWrite(LED1,0)',50);
};

setInterval(test, 200);
setInterval(blink, 5000);

