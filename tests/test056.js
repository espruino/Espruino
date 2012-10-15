// test for code that assert failed
var moveServo = function (p) {
 var pulses = 20;
 var f = function() { digitalPulse(D0,p); if (pulses-->0) setTimeout(f, 50); };
 f();
};

moveServo(1);
result=1;

